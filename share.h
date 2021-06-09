#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <pthread.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>
#include <time.h>
#include <semaphore.h>

#define LISTSIZ 50 
#define SHMKEY (key_t) 60128
#define SEMKEY (key_t) 60129
#define BILLION 100000000L;

int semid;

union semun {
	int val;
	struct semid_ds* buf;
	unsigned short int* aray;
}arg;

struct sembuf p = { 0,-1,0 };
struct sembuf v = { 0,1,0 };

void getsem(void) {
	if ((semid = semget(SEMKEY, 1, IPC_CREAT | 0666)) == -1) {
		perror("semget failed");
		exit(1);
	}
	arg.val = 1;
	if (semctl(semid, 0, SETVAL, arg) == -1) {
		perror("semctl failed");
		exit(1);
	}
}