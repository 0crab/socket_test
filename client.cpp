#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/unistd.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <thread>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <string>
#include "config.h"

using namespace std;

uint64_t buf_len;

uint64_t THREAD_NUM;

volatile int start = 0;

volatile int stop = 0;

uint64_t * sendbytes_list;

uint64_t * port_list;

string server_ip = "127.0.0.1";




unsigned long getRunTime(struct timeval begTime) {
    struct timeval endTime;
    gettimeofday(&endTime, NULL);
    unsigned long duration = (endTime.tv_sec - begTime.tv_sec) * 1000000 + endTime.tv_usec - begTime.tv_usec;
    return duration;
}

void timer(){
    __sync_fetch_and_add(&start,1);
    sleep(30);
    __sync_fetch_and_add(&stop,1);
}

void worker(int tid,int sfd){
    char * send_buf;
    send_buf = (char * )malloc(buf_len);
    memset(send_buf,1,sizeof(send_buf));
    while(!start){}
    while(!stop){
        write(sfd,send_buf,buf_len);
        sendbytes_list[tid]+=buf_len;
    }
}


int main(int argc, char **argv){
    if(argc == 3){
        THREAD_NUM = std::atol(argv[1]);
        buf_len = std::atol(argv[2]);
    }else{
        printf("client <thread_num>  <buf_len>\n");
        return 0;
    }

    port_list  = (uint64_t*)malloc(THREAD_NUM * sizeof(uint64_t));
    for(int i = 0 ; i < THREAD_NUM ; i++){
        port_list[i] = PORT_BASE +1;
    }

    sendbytes_list = (uint64_t*) malloc(THREAD_NUM * sizeof(uint64_t));

    int *fds = (int *)malloc(THREAD_NUM * sizeof(int));
    memset(fds,0,THREAD_NUM);

    for(int i = 0; i < THREAD_NUM; i++){
        socklen_t connect_fd;
        connect_fd = socket(AF_INET, SOCK_STREAM, 0);
        if(connect_fd < 0) {
            perror("cannot create communication socket");
            return 1;
        }

        struct sockaddr_in srv_addr;
        srv_addr.sin_family = AF_INET;
        srv_addr.sin_port = htons(port_list[i]);
        srv_addr.sin_addr.s_addr = inet_addr(server_ip.c_str());


        //connect server
        socklen_t ret;
        ret = connect(connect_fd, (struct sockaddr*)&srv_addr, sizeof(srv_addr));
        if(ret == -1) {
            perror("cannot connect to the server");
            close(connect_fd);
            return 1;
        }

        fds[i] = connect_fd;

    }

    vector<thread> threads;
    for(int i =0 ; i< THREAD_NUM;i++){
        threads.push_back(thread(worker,i,fds[i]));
    }

    thread a(timer);
    a.join();

    for(int i =0 ; i< THREAD_NUM;i++){
        threads[i].join();
    }

    uint64_t total_bytes = 0 ;
    for(int i =0 ;i<THREAD_NUM ;i++) total_bytes += sendbytes_list[i];

    cout<<"thread_num: "<<THREAD_NUM<<endl;
    cout<<"buf_len: "<<buf_len<<endl;
    cout<<"throughout: "<<total_bytes/(30*1000000)<<endl;

}
