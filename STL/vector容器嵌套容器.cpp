#include <iostream>
#include <vector>
using namespace std;

int main(){
    vector<vector<int>> b_array;
    vector<int> v1;
    vector<int> v2;
    vector<int> v3;
    vector<int> v4;

    for(int i = 0; i < 4; i++){
        v1.push_back(i+1);
        v2.push_back(i+2);
        v3.push_back(i+3);
        v4.push_back(i+4);
    }

    b_array.push_back(v1);
    b_array.push_back(v2);
    b_array.push_back(v3);
    b_array.push_back(v4);

    for(vector<vector<int>>::iterator it = b_array.begin(); it != b_array.end(); it++){
        for(vector<int>::iterator vit = (*it).begin(); vit != (*it).end(); vit++){
            cout << (*vit) << " ";
        }
        cout << endl;
    }
}