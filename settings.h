#define UNIX_DOMAIN "/home/czl/UNIX.sock"

#define TARGET_PORT 8883

#define  NUM_THREADS 4

#define SINGLE_BUF_SIZE  40
#define BATCH_NUM 100
#define NOBATCH_PACKAGE_NUM 1000000
#define BATCH_PACKAGE_NUM 100000

#define DO_UNIX 1
#define DO_BATCH 0

int server_port[4] = {8883, 8884, 8885, 8886};

char * unix_domain_list[4]={"/home/czl/work/socket_test/test1.sock",\
                            "/home/czl/work/socket_test/test2.sock",\
                            "/home/czl/work/socket_test/test3.sock",\
                            "/home/czl/work/socket_test/test4.sock"
                        };