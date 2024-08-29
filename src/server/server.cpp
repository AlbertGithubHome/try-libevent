#include <iostream>
#include <event2/event.h>

int main() {
    struct event_base *base = event_base_new();
    if (!base) {
        std::cerr << "Could not initialize libevent" << std::endl;
        return 1;
    }

    std::cout << "Hello from server" << std::endl;

    event_base_free(base);
    return 0;
}
