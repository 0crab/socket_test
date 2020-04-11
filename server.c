#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/unistd.h>
#include <netinet/in.h>
#include "settings.h"



#define READ_FROM_CLIENT 0X01
#define WRITE_TO_CLIENT 0x02

void read_nobatch(socklen_t fd);
void read_inbatch(socklen_t fd);

int main(void)
{
    if(DO_UNIX){
        printf("PF_UNIX\n");
        socklen_t listen_fd;
        listen_fd = socket(PF_UNIX, SOCK_STREAM, 0);
        if(listen_fd < 0) {
            perror("cannot create communication socket");
            return 1;
        }
        struct sockaddr_un srv_addr;
        srv_addr.sun_family = AF_UNIX;
        strcpy(srv_addr.sun_path,UNIX_DOMAIN);
        socklen_t ret;
        ret = bind(listen_fd, (struct sockaddr*)&srv_addr, sizeof(srv_addr));
        if(ret == -1) {
            perror("cannot bind server socket");
            close(listen_fd);
            unlink(UNIX_DOMAIN);
            return 1;
        }
        ret = listen(listen_fd,1);
        if(ret == -1) {
            perror("cannot listen the client connect request");
            close(listen_fd);
            unlink(UNIX_DOMAIN);
            return 1;
        }

        socklen_t con_fd;
        socklen_t len;

        while(1) {
            //have connect request use accept
            len = sizeof(srv_addr);
            con_fd = accept(listen_fd, (struct sockaddr *) &srv_addr, &len);
            if (con_fd < 0) {
                perror("cannot accept client connect request");
                close(listen_fd);
                return 1;
            }
            if (DO_BATCH) {
                read_inbatch(con_fd);
            } else {
                read_nobatch(con_fd);
            }

        }

        }else{
        printf("AF_INET\n");
        socklen_t listen_fd;
        listen_fd = socket(AF_INET, SOCK_STREAM, 0);
        if(listen_fd < 0) {
            perror("cannot create communication socket");
            return 1;
        }

        struct sockaddr_in srv_addr;
        srv_addr.sin_family = AF_INET;
        srv_addr.sin_port = htons(SERVER_PORT);
        srv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

        socklen_t ret;
        ret = bind(listen_fd, (struct sockaddr*)&srv_addr, sizeof(srv_addr));
        if(ret == -1) {
            perror("cannot bind server socket");
            close(listen_fd);
            return 1;
        }

        ret = listen(listen_fd,1);
        if(ret == -1) {
            perror("cannot listen the client connect request");
            close(listen_fd);
            return 1;
        }
        socklen_t con_fd;
        socklen_t len;

        while(1) {
            //have connect request use accept
            len = sizeof(srv_addr);
            con_fd = accept(listen_fd,(struct sockaddr*)&srv_addr,&len);
            if(con_fd < 0) {
                perror("cannot accept client connect request");
                close(listen_fd);
                return 1;
            }
            if(DO_BATCH){
                read_inbatch(con_fd);
            }else{
                read_nobatch(con_fd);
            }

        }

    }
}

void read_nobatch(socklen_t fd){
    printf("read_nobatch\n");
    int read_rt;
    char rec_buf[SINGLE_BUF_SIZE];
    int count=0;
    for(int i=0;i<NOBATCH_PACKAGE_NUM;i++){
        memset(rec_buf,0,SINGLE_BUF_SIZE);
        read_rt=read(fd,rec_buf,SINGLE_BUF_SIZE);
        //printf("rec %d bytes:%s\n",read_rt,rec_buff);
        if(read_rt==SINGLE_BUF_SIZE)
            count++;
    }
    printf("receive:%d\n",count);
}

void read_inbatch(socklen_t fd){
    printf("read_inbatch\n");
    int read_rt;
    char rec_buf[SINGLE_BUF_SIZE*BATCH_NUM];
    int count=0;
    for(int i=0;i<BATCH_PACKAGE_NUM;i++){
        memset(rec_buf,0,SINGLE_BUF_SIZE*BATCH_NUM);
        read_rt=read(fd,rec_buf,SINGLE_BUF_SIZE*BATCH_NUM);
        //printf("rec %d bytes:%s\n",read_rt,rec_buff);
        if(read_rt==SINGLE_BUF_SIZE*BATCH_NUM)
            count++;
    }
    printf("receive:%d\n",count);
}