#include <stdbool.h>

#include "app.h"

int main(void)
{
    app_init();

    while (true)
    {
        app_task();
    }

    return 0;
}