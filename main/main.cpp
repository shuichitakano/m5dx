/*
 * author : Shuichi TAKANO
 * since  : Sun Jan 06 2019 20:43:25
 */

#include <sys/util.h>
#include <stdio.h>
#include <io/sd.h>

#include <dirent.h>
#include <io/file_util.h>
#include <vector>

extern "C" void
app_main()
{
    io::initializeSD("/sdcard");

    {
        DIR *dp;
        struct dirent *ep;
        dp = opendir("/sdcard/");

        if (dp != NULL)
        {
            while ((ep = readdir(dp)))
                puts(ep->d_name);

            (void)closedir(dp);
        }
    }

#if 0
    FILE *fp = fopen("/sdcard/test.txt", "r");
    if (fp)
    {
        static char test[1024] = {};
        fread(test, 1024, 1, fp);
        fclose(fp);
        printf("str = %s\n", test);
    }
    else
    {
        printf("file open error\n");
    }
#endif

    {
        auto size = io::getFileSize("/sdcard/test.txt");
        printf("size = %d\n", size);

        std::vector<uint8_t> buf;
        io::readFile(buf, "/sdcard/test.txt");
        buf.push_back(0);
        printf("file = %s\n", buf.data());
    }

    while (1)
    {
        static int count = 0;
        printf("count = %d\n", count++);
        sys::delay(1000);
    }
}