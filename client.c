#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/unistd.h>
#include <netinet/in.h>
#include <sys/time.h>
#include "settings.h"
//#include "tracer.h"


void send_inbatch(socklen_t fd);
void send_nobatch(socklen_t fd);


struct timeval starttime;
unsigned long runtime;

unsigned long getRunTime(struct timeval begTime) {
    struct timeval endTime;
    gettimeofday(&endTime, NULL);
    unsigned long duration = (endTime.tv_sec - begTime.tv_sec) * 1000000 + endTime.tv_usec - begTime.tv_usec;
    return duration;
}


int main(void)
{


    if(DO_UNIX){
        printf("PF_UNIX\n");
        //create unix socket
        socklen_t connect_fd;
        connect_fd = socket(PF_UNIX, SOCK_STREAM, 0);
        if(connect_fd < 0) {
            perror("cannot create communication socket");
            return 1;
        }

        struct sockaddr_un srv_addr;
        srv_addr.sun_family = AF_UNIX;
        strcpy(srv_addr.sun_path,UNIX_DOMAIN);

        //connect server
        socklen_t ret;
        ret = connect(connect_fd, (struct sockaddr*)&srv_addr, sizeof(srv_addr));
        if(ret == -1) {
            perror("cannot connect to the server");
            close(connect_fd);
            return 1;
        }

        if(DO_BATCH){
            send_inbatch(connect_fd);
        }else{
            send_nobatch(connect_fd);
        }

    }else{
        printf("AF_INET\n");
        //create unix socket
        socklen_t connect_fd;
        connect_fd = socket(AF_INET, SOCK_STREAM, 0);
        if(connect_fd < 0) {
            perror("cannot create communication socket");
            return 1;
        }

        struct sockaddr_in srv_addr;
        srv_addr.sin_family = AF_INET;
        srv_addr.sin_port = htons(SERVER_PORT);
        srv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

        //connect server
        socklen_t ret;
        ret = connect(connect_fd, (struct sockaddr*)&srv_addr, sizeof(srv_addr));
        if(ret == -1) {
            perror("cannot connect to the server");
            close(connect_fd);
            return 1;
        }

        if(DO_BATCH){
            send_inbatch(connect_fd);
        }else{
            send_nobatch(connect_fd);
        }


    }

}

void send_inbatch(socklen_t fd){
    printf("send_inbatch\npackagenum:%d batchnum:%d\n",BATCH_PACKAGE_NUM,BATCH_NUM);
    char send_buf[SINGLE_BUF_SIZE*BATCH_NUM];
    memset(send_buf,1,SINGLE_BUF_SIZE*BATCH_NUM);
    gettimeofday(&starttime,NULL);
    for(int i=0;i<BATCH_PACKAGE_NUM;i++){
        write(fd, send_buf, SINGLE_BUF_SIZE*BATCH_NUM);
    }
    runtime=getRunTime(starttime);
    printf("runtime:%lu\n",runtime);
    close(fd);
}

void send_nobatch(socklen_t fd){
    printf("send_nobatch\npackagenum:%d\n",NOBATCH_PACKAGE_NUM);
    char send_buf[SINGLE_BUF_SIZE];
    memset(send_buf,1,SINGLE_BUF_SIZE);
    gettimeofday(&starttime,NULL);
    for(int i=0;i<NOBATCH_PACKAGE_NUM;i++){
        write(fd, send_buf, SINGLE_BUF_SIZE);
    }
    runtime=getRunTime(starttime);
    printf("runtime:%lu\n",runtime);
    close(fd);
}