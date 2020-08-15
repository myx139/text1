#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

/*
    argc:参数个数
    *argv[]:具体参数
*/
int main(int argc,char *argv[])
{
    int ret = 0;
    int fd  = 0;
    char readbuf[100];
    char writebuf[100];
    char *filename = argv[1];
/*文件打开函数*/
    fd = open(filename, O_RDWR);
    if (fd < 0)
    {
        printf("Can't open this file!\r\n");
    }
    else
    {
        printf("Ok!\r\n");
    }
    
/*文件读函数*/
    ret = read(fd,readbuf, 50);
    if (ret < 0)
    {
        printf("Read failed!\r\n");
    }
    else
    {
        /* code */
    }
/*文件写函数*/
    ret = write(fd, writebuf, 50);
    if(ret < 0)
    {
        printf("Write failed!\r\n");
    }
    else
    {
        /* code */
    }
/*文件关闭*/
    ret = close(fd);
    if (ret < 0)
    {
        printf("Can't close this file!\r\n");
    }
    
    return 0;
}