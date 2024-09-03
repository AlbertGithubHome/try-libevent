#include <iostream>
#include "AWNet.h"

int main()
{
    int port = 8808;
    AWNet* pNet = new AWNet();
    pNet->Initialization(100, port);

    std::cout << "Hello from server with port [" << port <<"]" << std::endl;

    while (true)
    {
        pNet->Execute();
    }

    pNet->Final();

    std::cout << "Bye from server" << std::endl;

    return 0;
}
