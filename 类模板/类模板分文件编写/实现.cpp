#include <iostream>
using namespace std;
#include "Person.hpp"

int main(){
    Person<int, string> P1(22,"YOCO");
    P1.showInfo();
}