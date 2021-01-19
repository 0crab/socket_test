#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/unistd.h>
#include <netinet/in.h>
#include <thread>
#include <iostream>
#include <vector>
#include "config.h"

using namespace  std;

int thread_num;
int rec_buf_size;
#define THREAD_NUM thread_num
#define REC_BUF_SIZE rec_buf_size

void get_listen_fd(int tid,bool rec_port,socklen_t & listen_fd,struct sockaddr_in &srv_addr){

    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) {
        perror("cannot create communication socket");
        return;
    }

    int port = rec_port? PORT_BASE + tid : PORT_BASE + THREAD_NUM + tid;
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_port = htons(port);
    srv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    socklen_t ret;
    ret = bind(listen_fd, (struct sockaddr *) &srv_addr, sizeof(srv_addr));
    if (ret == -1) {
        perror("cannot bind server socket");
        close(listen_fd);
        return;
    }

    ret = listen(listen_fd, 1);
    if (ret == -1) {
        perror("cannot listen the client connect request");
        close(listen_fd);
        return;
    }
}

void worker(int tid) {

    char rec_buf[REC_BUF_SIZE];

    socklen_t  rec_listen_fd;
    struct sockaddr_in rec_srv_addr;
    get_listen_fd(tid,true,rec_listen_fd,rec_srv_addr);
    socklen_t  send_listen_fd;
    struct sockaddr_in send_srv_addr;
    get_listen_fd(tid,false,send_listen_fd,send_srv_addr);

    socklen_t con_fd1,con_fd2;
    socklen_t len;

    while (1) {
        //have connect request use accept
        len = sizeof(rec_srv_addr);
        con_fd1 = accept(rec_listen_fd, (struct sockaddr *) &rec_srv_addr, &len);
        if (con_fd1 < 0) {
            perror("cannot accept client connect request");
            close(rec_listen_fd);
            return;
        }
        cout<<tid + PORT_BASE<<" rec a connect"<<endl;

        len = sizeof(send_srv_addr);
        con_fd2 = accept(send_listen_fd, (struct sockaddr *) &send_srv_addr, &len);
        if (con_fd2 < 0) {
            perror("cannot accept client connect request");
            close(send_listen_fd);
            return;
        }
        cout<<tid + PORT_BASE + THREAD_NUM<<" rec a connect"<<endl;



        uint64_t count = 0 ;
        while (read(con_fd1, rec_buf, REC_BUF_SIZE) > 0) {
            for(int i = 0; i < REC_BUF_SIZE; i ++ ) count +=(uint8_t )rec_buf[i];
            write(con_fd2,rec_buf,REC_BUF_SIZE);
        }
        cout<<tid<<" recved "<<count<<endl;
    }
}


int main(int argc, char **argv) {
    if(argc == 3){
        THREAD_NUM = std::atol(argv[1]);
        REC_BUF_SIZE = std::atol(argv[2]);
    }else{
        printf("client <thread_num>  <rec_buf_size>\n");
        return 0;
    }
    cout<<"thread_num "<<thread_num<<"rec_buf_size"<<rec_buf_size<<endl;
    vector<thread> threads;
    for (int i = 0; i < THREAD_NUM; i++) threads.push_back(thread(worker,i));
    for (int i = 0; i < THREAD_NUM; i++) threads[i].join();
}