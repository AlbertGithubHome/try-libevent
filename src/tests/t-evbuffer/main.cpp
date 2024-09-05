#include <event2/buffer.h>
#include <iostream>
#include <string>

int main() {
    // 创建一个新的 evbuffer
    struct evbuffer* buffer = evbuffer_new();
    if (!buffer) {
        std::cerr << "Failed to create evbuffer" << std::endl;
        return 1;
    }

    // 要添加到缓冲区的字符串
    const std::string data1 = "Hello, ";
    const std::string data2 = "World!";

    // 将字符串 data1 和 data2 添加到缓冲区
    evbuffer_add(buffer, data1.data(), data1.size());
    evbuffer_add(buffer, data2.data(), data2.size());

    // 打印缓冲区当前的长度
    std::cout << "Buffer length after adding data: " << evbuffer_get_length(buffer) << std::endl;

    // 读取并打印缓冲区的内容
    char output[50];  // 输出缓冲区
    size_t data_len = evbuffer_remove(buffer, output, sizeof(output) - 1);

    if (data_len > 0) {
        output[data_len] = '\0';  // 添加字符串结束符
        std::cout << "Data read from buffer: " << output << std::endl;
    } else {
        std::cout << "No data in buffer to read." << std::endl;
    }

    // 再次打印缓冲区长度（应该为 0）
    std::cout << "Buffer length after reading data: " << evbuffer_get_length(buffer) << std::endl;

    // 释放 evbuffer
    evbuffer_free(buffer);

    return 0;
}
