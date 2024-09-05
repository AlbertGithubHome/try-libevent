#include "common/AWNet.h"
#include <iostream>
#include <thread>

int main()
{
    int port = 8808;
    AWNet* pNet = new AWNet();
    pNet->Initialization(1024, port);

    std::cout << "Hello from server with port [" << port <<"]" << std::endl;

    AWINT64 now = AWGetTimeMS();

    while (true)
    {
        pNet->Execute();

        AWINT64 ts = AWGetTimeMS();
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
