#include "common/AWNet.h"
#include <iostream>
#include <chrono>
#include <thread>

int main()
{
    int port = 8808;
    AWNet* pNet = new AWNet();
    pNet->Initialization(1024, port);

    std::cout << "Hello from server with port [" << port <<"]" << std::endl;

    AWINT64 now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

    while (true)
    {
        pNet->Execute();

        AWINT64 ts = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        if (ts - now >= 60000)
        {
            std::cout << "tick[" << ts << "]" << std::endl;
            now = ts;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    pNet->Final();

    std::cout << "Bye from server" << std::endl;

    return 0;
}
