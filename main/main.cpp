/*
 * author : Shuichi TAKANO
 * since  : Sun Jan 06 2019 20:43:25
 */

#include <sys/util.h>
#include <stdio.h>

extern "C" void
app_main()
{
    while (1)
    {
        static int count = 0;
        printf("count = %d\n", count++);
        sys::delay(1000);
    }
}