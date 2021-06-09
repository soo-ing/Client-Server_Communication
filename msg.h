#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h> 
#include <sys/msg.h> 
#include <sys/stat.h> 
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define STDIN 0
#define LISTSIZ 50
#define BILLION 100000000L;

#define SHMKEY1 (key_t) 60121 
#define SHMKEY2 (key_t) 60122 
#define SHMKEY3 (key_t) 60123 

struct message {
	long mtype;
	char mtext[LISTSIZ + 1];
};
struct sendbuf
{
	long msgtype; //클라이언트 pid 값으로 보냄
	int arr[LISTSIZ];
};
struct recvbuf
{
	long msgtype; // 서버 pid 값으로 받음
	int data;
	int sss;
	int clnt_pid; // 클라이언트 pid
};
struct resultbuf
{
	long msgtype; // 클라이언트 pid 값으로 보냄
	int result; // 서버에서 클라이언트로 보내는 요청 결과 값
};
typedef struct Node
{
	int pid;
	struct Node* link;
}Node;