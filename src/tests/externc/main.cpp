#include <iostream>
#include "./foo.h"

void fun(int a, double b) {
    std::cout << "C++ function:" << a << " " << b << std::endl;
}

int main() {
    fun(2, 3); // 调用 C 函数
    return 0;
}
