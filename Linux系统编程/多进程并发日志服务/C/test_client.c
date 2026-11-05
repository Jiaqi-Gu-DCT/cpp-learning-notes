#define _GNU_SOURCE
#include "log_client.h"
#include <sys/wait.h>
#include <sys/time.h>
#include <stdlib.h>

// 测试配置
typedef struct {
    int client_num;     // 客户端进程数
    int log_per_client; // 每个客户端写入条数
    int nonblock_mode;  // 是否启用非阻塞模式
    int verbose;        // 是否打印详细日志
} TestConfig;

// 每个子进程的测试结果
typedef struct {
    int success_cnt;
    int fail_cnt;
    long long cost_us;  // 总耗时(微秒)
} TestResult;

// 子进程：执行单客户端测试
void run_client_worker(int id, TestConfig *config, TestResult *result) {
    LogClient client;
    char proc_name[32];
    snprintf(proc_name, sizeof(proc_name), "Worker-%d", id);

    if (log_connect(&client, proc_name) != 0) {
        fprintf(stderr, "[Worker-%d] 连接日志服务失败\n", id);
        result->success_cnt = 0;
        result->fail_cnt = config->log_per_client;
        return;
    }

    struct timeval start, end;
    gettimeofday(&start, NULL);

    for (int i = 0; i < config->log_per_client; i++) {
        LogLevel level = (LogLevel)(i % 4);
        int ret;

        if (config->nonblock_mode) {
            ret = log_write_nb(&client, level,
                "并发压测日志 #%d | 客户端ID:%d | 负载测试数据填充abcdefghijklmnopqrstuvwxyz",
                i + 1, id);
        } else {
            ret = log_write(&client, level,
                "并发压测日志 #%d | 客户端ID:%d | 负载测试数据填充abcdefghijklmnopqrstuvwxyz",
                i + 1, id);
        }

        if (ret == 0) {
            result->success_cnt++;
        } else {
            result->fail_cnt++;
        }

        // 非压测模式下加少量间隔，模拟真实业务节奏
        if (!config->nonblock_mode && config->verbose) {
            usleep(1000);
        }
    }

    gettimeofday(&end, NULL);
    result->cost_us = (end.tv_sec - start.tv_sec) * 1000000LL 
                    + (end.tv_usec - start.tv_usec);

    log_disconnect(&client);
}

int main(int argc, char *argv[]) {
    // 默认配置
    TestConfig config = {
        .client_num = 5,
        .log_per_client = 500,
        .nonblock_mode = 0,
        .verbose = 0
    };

    // 简单命令行参数解析
    if (argc >= 2) config.client_num = atoi(argv[1]);
    if (argc >= 3) config.log_per_client = atoi(argv[2]);
    if (argc >= 4) config.nonblock_mode = atoi(argv[3]);

    printf("========== 多进程日志服务并发测试 ==========\n");
    printf("客户端数量: %d\n", config.client_num);
    printf("单客户端写入: %d 条\n", config.log_per_client);
    printf("写入模式: %s\n\n", config.nonblock_mode ? "非阻塞(缓冲区满丢弃)" : "阻塞(等待空槽位)");

    // 开辟共享内存存结果（子进程写完父进程可读）
    int shm_res = shmget(IPC_PRIVATE, sizeof(TestResult) * config.client_num, IPC_CREAT | 0666);
    TestResult *results = (TestResult *)shmat(shm_res, NULL, 0);
    memset(results, 0, sizeof(TestResult) * config.client_num);

    pid_t *pids = malloc(sizeof(pid_t) * config.client_num);
    struct timeval global_start, global_end;
    gettimeofday(&global_start, NULL);

    // 创建所有客户端子进程
    for (int i = 0; i < config.client_num; i++) {
        pids[i] = fork();
        if (pids[i] == -1) {
            perror("fork");
            exit(1);
        }
        if (pids[i] == 0) {
            run_client_worker(i, &config, &results[i]);
            exit(0);
        }
    }

    // 等待所有子进程结束
    for (int i = 0; i < config.client_num; i++) {
        waitpid(pids[i], NULL, 0);
    }

    gettimeofday(&global_end, NULL);
    long long total_cost_us = (global_end.tv_sec - global_start.tv_sec) * 1000000LL
                            + (global_end.tv_usec - global_start.tv_usec);

    // 汇总统计
    int total_success = 0, total_fail = 0;
    for (int i = 0; i < config.client_num; i++) {
        total_success += results[i].success_cnt;
        total_fail += results[i].fail_cnt;
    }

    int total = total_success + total_fail;
    double qps = (double)total_success / (total_cost_us / 1000000.0);// 吞吐量：每秒处理请求数
    double drop_rate = total == 0 ? 0 : (double)total_fail / total * 100;// 丢包率=（写入失败的日志数÷总请求数）

    printf("\n================ 测试结果 ================\n");
    printf("总请求数:     %d\n", total);
    printf("写入成功:     %d\n", total_success);
    printf("丢弃失败:     %d\n", total_fail);
    printf("丢包率:       %.2f%%\n", drop_rate);
    printf("总耗时:       %.3f 秒\n", total_cost_us / 1000000.0);
    printf("系统吞吐量:   %.0f 条/秒\n", qps);
    printf("==========================================\n");
    printf("查看日志文件: tail -f %s\n", LOG_FILE);

    // 资源清理
    free(pids);
    shmdt(results);
    shmctl(shm_res, IPC_RMID, NULL);
    return 0;
}
