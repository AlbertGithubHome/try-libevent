#include "common/AWNet.h"
#include <iostream>
#include <chrono>
#include <thread>

int main()
{
    int port = 8808;
    std::string ip = "127.0.0.1";
    AWNet* pNet = new AWNet();
    pNet->Initialization(ip.c_str(), port);

    std::cout << "Hello client to connect to [" << ip << ":" << port <<"]." << std::endl;

    // this is valid because the connect is not connected
    pNet->SendMsg("Hello", 0);

    AWINT64 now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

    while (true)
    {
        pNet->Execute();

        AWINT64 ts = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        if (ts - now >= 10000)
        {
            std::cout << "tick[" << ts << "]" << std::endl;
            now = ts;

            pNet->SendMsg("msg tick " + std::to_string(ts), 0);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    pNet->Final();

    std::cout << "Bye from client" << std::endl;

    return 0;
}
