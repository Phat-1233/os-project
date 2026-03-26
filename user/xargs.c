#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/param.h"
#include "user/user.h"

int main(int argc, char *argv[])
{
    char buf[512];
    char *args[MAXARG];
    int i;

    for (i = 1; i < argc; i++)
    {
        args[i - 1] = argv[i];
    }

    int base = argc - 1;
    int n = 0;
    char c;

    while (read(0, &c, 1) == 1)
    {
        if (c == '\n')
        {
            buf[n] = 0;

            args[base] = buf;
            args[base + 1] = 0;

            if (fork() == 0)
            {
                exec(args[0], args);
                exit(1);
            }

            wait(0);
            n = 0;
        }
        else
        {
            if (n < sizeof(buf) - 1)
            {
                buf[n++] = c;
            }
        }
    }

    if (n > 0)
    {
        buf[n] = 0;

        args[base] = buf;
        args[base + 1] = 0;

        if (fork() == 0)
        {
            exec(args[0], args);
            exit(1);
        }

        wait(0);
    }

    exit(0);
}