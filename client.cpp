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
#include "tracer.h"
#include "config.h"

using namespace std;

uint64_t send_buf_len;
uint64_t thread_num;
uint64_t timer_range;
#define THREAD_NUM thread_num
#define SEND_BUF_LEN send_buf_len
#define TIMER_RANGE timer_range

#define ROUND_SET 10000000

uint64_t  g_sendcount;
uint64_t * runtimelist;

std::atomic<int> stopMeasure(0);

string server_ip = "127.0.0.1";


void worker(int tid,int sfd){

    char * send_buf;
    send_buf = (char * )malloc(send_buf_len);
    memset(send_buf,1,sizeof(send_buf));

    Tracer t;
    t.startTime();
    while (stopMeasure.load(std::memory_order_relaxed) == 0) {
        for(size_t i = 0; i < ROUND_SET; i++){
            write(sfd,send_buf,send_buf_len);
        }
        __sync_fetch_and_add(&g_sendcount, ROUND_SET);

        uint64_t tmptruntime = t.fetchTime();
        if (tmptruntime / 1000000 >= timer_range) {
            stopMeasure.store(1, memory_order_relaxed);
        }
    }
    runtimelist[tid] = t.getRunTime();
}


int get_connect_fd(string target_ip,int port){
    socklen_t connect_fd;
    connect_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(connect_fd < 0) {
        perror("cannot create communication socket");
        return 1;
    }

    struct sockaddr_in srv_addr;
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_port = htons(port);
    srv_addr.sin_addr.s_addr = inet_addr(target_ip.c_str());


    //connect server
    socklen_t ret;
    ret = connect(connect_fd, (struct sockaddr*)&srv_addr, sizeof(srv_addr));
    if(ret == -1) {
        perror("cannot connect to the server");
        close(connect_fd);
        return 1;
    }
    return connect_fd;
}

int main(int argc, char **argv){
    if(argc == 4){
        THREAD_NUM = std::atol(argv[1]);
        SEND_BUF_LEN = std::atol(argv[2]);
        TIMER_RANGE =   std::atol(argv[3]);
    }else{
        printf("client <thread_num>  <send_buf_len> <timer_range>\n");
        return 0;
    }
    cout<<"thread_num "<<thread_num<<"send_buf_len "<<send_buf_len<<"timer_range "<<timer_range<<endl;

    runtimelist = new uint64_t [THREAD_NUM]();

    int *fds = new int[THREAD_NUM]();
    for(int i = 0; i < THREAD_NUM; i++){
        fds[i] = get_connect_fd(server_ip,PORT_BASE + i);
    }

    vector<thread> threads;
    for(int i =0 ; i< THREAD_NUM;i++) threads.push_back(thread(worker,i,fds[i]));
    for(int i =0 ; i< THREAD_NUM;i++) threads[i].join();

    uint64_t runtime = 0;
    for (int i = 0; i < thread_num; i++)  runtime += runtimelist[i];
    runtime /= thread_num;
    cout << " runtime " << runtime << endl;

    double throughput = g_sendcount * 1.0 / runtime;
    cout << "***count_throughput " << throughput << endl;
    cout << "***byte_throughput "<<throughput * send_buf_len<<endl;


}
