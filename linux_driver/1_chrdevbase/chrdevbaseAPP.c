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
    char readbuffer[100];
    char writebuffer[100];
    static char usrdata[] = {"Hello Linux!"};
    if(argc != 3)
    {
        printf("ERROR\r\n");
        return -1;
    }
    
    filename  = argv[1];
    fd = open(filename,O_RDWR);
    if(fd < 0)
    {
        printf("can't open file %s\r\n",filename);
        return -1;
    }
    if(atoi(argv[2]) == 1) //读
    {
        ret = read(fd, readbuffer, 50);
        if(ret < 0)
            {
            printf("can't read file %s\r\n",filename);
            }
        else
            {
            printf("read data :%s\r\n",readbuffer);
            }
    }
    if(atoi(argv[2]) == 2) //写
    {
        memcpy(writebuffer,usrdata,sizeof(usrdata));
        ret = write(fd, writebuffer, 50);
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
