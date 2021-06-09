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
	long msgtype; //Ŭ���̾�Ʈ pid ������ ����
	int arr[LISTSIZ];
};
struct recvbuf
{
	long msgtype; // ���� pid ������ ����
	int data;
	int sss;
	int clnt_pid; // Ŭ���̾�Ʈ pid
};
struct resultbuf
{
	long msgtype; // Ŭ���̾�Ʈ pid ������ ����
	int result; // �������� Ŭ���̾�Ʈ�� ������ ��û ��� ��
};
typedef struct Node
{
	int pid;
	struct Node* link;
}Node;