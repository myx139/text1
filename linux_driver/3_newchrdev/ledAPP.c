#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>


int main(int argc,char *argv[])
{
    int ret = 0;
    int fd = 0;
    char *filename;
    static int readbuffer[1];
    char writebuffer[1];
    int databuffer[1];
    /*
    if(argc != 4)
    {
        printf("ERROR\r\n");
        return -1;
    }
    */
    
    filename  = argv[1];
    fd = open(filename,O_RDWR);
    if(fd < 0)
    {
        printf("can't open file %s\r\n",filename);
        return -1;
    }
    if(*argv[2] == 'r') //读
    {
        ret = read(fd, readbuffer, 1);
        if(ret < 0)
            {
            printf("can't read file %s\r\n",filename);
            }
        else
            {
                if(readbuffer[0] == 0)
                {
                    printf("LED为打开状态！\r\n");
                }
                else if (readbuffer[0] == 1)
                {
                    printf("LED为关闭状态！\r\n");
                }
                
            }
    }
    if(*argv[2] == 'w') //写
    {
        databuffer[0] = atoi(argv[3]);
        memcpy(writebuffer,databuffer,sizeof(databuffer));
        ret = write(fd, writebuffer, 1);
        if(ret < 0)
            {
                printf("can't write file %s\r\n",filename);
            }
        else
            {
                /* code */
            }
    }
    ret = close(fd);
    if(ret < 0)
    {
        printf("can't close file %s\r\n",filename);
    }
    return 0;
}
