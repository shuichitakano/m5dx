/*
 * author : Shuichi TAKANO
 * since  : Sun Jan 06 2019 20:43:25
 */

#include <sys/util.h>
#include <stdio.h>
//#include <io/sd.h>

//#include <dirent.h>
#include <io/file_util.h>
#include <vector>

#include <SD.h>

extern "C" void
app_main()
{

#if 0
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
#endif

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

    int TFCARD_CS_PIN = 4;
    if (!SD.begin(TFCARD_CS_PIN, SPI, 40000000))
    {
        printf("initialize SD failed.\n");
    }

    {
        //        const char *filename = "/sdcard/test.txt";
        const char *filename = "/test.txt";

        auto size = io::getFileSize(filename);
        printf("size = %d\n", size);

        std::vector<uint8_t> buf;
        io::readFile(buf, filename);
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