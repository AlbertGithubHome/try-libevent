#include <event2/event.h>
#include <stdio.h>

int main() {
    struct event_base *base = event_base_new();
    const char *method = event_base_get_method(base);
    printf("Libevent is using: %s\n", method);
    event_base_free(base);
    return 0;
}
