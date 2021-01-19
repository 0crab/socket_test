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

void worker(int tid) {

    char rec_buf[REC_BUF_SIZE];

    socklen_t listen_fd;
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) {
        perror("cannot create communication socket");
        return;
    }

    struct sockaddr_in srv_addr;
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_port = htons(PORT_BASE+tid);
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
    socklen_t con_fd;
    socklen_t len;

    while (1) {
        //have connect request use accept
        len = sizeof(srv_addr);
        con_fd = accept(listen_fd, (struct sockaddr *) &srv_addr, &len);
        if (con_fd < 0) {
            perror("cannot accept client connect request");
            close(listen_fd);
            return;
        }
        cout<<tid + PORT_BASE<<" rec a connect"<<endl;
        while (read(con_fd, rec_buf, REC_BUF_SIZE) > 0) {}
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