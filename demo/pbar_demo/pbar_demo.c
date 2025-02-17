#include "allay/pbar/pbar.h"

#include <stdio.h>

static void common_sleep(int ms) {
#ifdef _WIN32
    Sleep((DWORD)ms);
#else
    usleep(ms * 1000);
#endif
}

int main(void) {
    pbar demo;
    pbar_init(&demo, 2.0, PCT_BAR_ARG_TIME);

    for (int i = 1; i <= 200; i++) {
        pbar_update(&demo, i / 100.0, 20);
        common_sleep(i / 3);
    }

    pbar demo2;

    pbar_init(&demo2, 1.0, PCT_TIME);

    for (int i = 1; i <= 100; i++) {
        pbar_update_newline(&demo2, i / 100.0, 5);
        common_sleep(i / 3);

        if (i % 10 == 0) { printf("%d\n", i); }
    }

    return 0;
}
