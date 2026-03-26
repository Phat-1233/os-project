#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fs.h"
#include "user/user.h"

void print_indent(int depth)
{
    for (int i = 0; i < depth; i++)
        printf("  ");
}

void tree(char *path, int depth)
{
    int fd;
    struct stat st;
    struct dirent de;
    char buf[512];
    char name[DIRSIZ + 1];

    if ((fd = open(path, 0)) < 0)
    {
        fprintf(2, "tree: cannot open %s\n", path);
        exit(1);
    }

    if (fstat(fd, &st) < 0)
    {
        fprintf(2, "tree: cannot stat %s\n", path);
        close(fd);
        exit(1);
    }

    if (st.type != T_DIR)
    {
        close(fd);
        return;
    }

    while (read(fd, &de, sizeof(de)) == sizeof(de))
    {
        if (de.inum == 0)
            continue;

        memmove(name, de.name, DIRSIZ);
        name[DIRSIZ] = 0;

        if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0)
            continue;

        if (strlen(path) + 1 + strlen(name) + 1 > sizeof(buf))
        {
            printf("tree: path too long\n");
            continue;
        }

        strcpy(buf, path);
        char *p = buf + strlen(buf);
        *p++ = '/';
        strcpy(p, name);

        if (stat(buf, &st) < 0)
        {
            printf("tree: cannot stat %s\n", buf);
            continue;
        }

        print_indent(depth);

        if (st.type == T_DIR)
        {
            printf("%s/\n", name);
            tree(buf, depth + 1);
        }
        else
        {
            printf("%s\n", name);
        }
    }

    close(fd);
}

int main(int argc, char *argv[])
{
    char *path;

    if (argc > 2)
    {
        fprintf(2, "usage: tree [directory]\n");
        exit(1);
    }

    if (argc == 1)
        path = ".";
    else
        path = argv[1];

    printf("%s/\n", path);

    tree(path, 1);

    exit(0);
}