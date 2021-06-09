#include "pipe.h"
#define BILLION 100000000L;

char msg[MSG_SIZE];
char snd[MSG_SIZE];
int fd0, fd1;
int nread, i;
int a, b;

sem_t sem;

void *writefunc(void *flag);
int readfunc();
void *data_read(void *sndmsg);
pthread_t p_thread[3];

int semid;

/* 세마포어 설정*/
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

int pipefunc() {   //pipe 읽어오는 함수
				   /*fd0은 서버가 읽고 클라가 쓰는용*/
	if ((fd0 = open(PIPENAME0, O_WRONLY)) < 0) {
		printf("fail to open named pipe0\n");
		return 0;
	}
	/* pipe 열기, Read Write가능 해야 한다 */
	/*fd1은 서버가 쓰고 클라가 읽는용*/
	if ((fd1 = open(PIPENAME1, O_RDONLY)) < 0) {
		printf("fail to open named pipe1\n");
		return 0;
	}
}



void *writefunc(void *flag) {   //pipe에 write하는 함수
	sem_wait(&sem);
	snprintf(snd, sizeof(snd), "%d", 1);
	if ((nread = write(fd0, snd, sizeof(snd))) < 0) {
		printf("fail to call write()\n");
	}

	sem_post(&sem);
}

void *data_read(void *arg) {   //데이터 읽어오기						
	sem_wait(&sem);

	for (i = 0; i < LISTSIZ; i++) {
		if ((nread = read(fd1, msg, sizeof(msg))) < 0) {
			printf("read failed\n");
		}
		printf("server: %s\n", msg);   //서버에서 보낸 데이터 출력
	}
	sem_post(&sem);
}

void main(void) {
	pipefunc();   //파이프 연결
	int status;
	int num= 0;
	sem_init(&sem, 0, 1);

	struct timespec start, stop;
	double accum;

	while (1) {
		scanf("%d", &num);
		if (clock_gettime(CLOCK_MONOTONIC, &start) == -1) {   //시간측정시작
			perror("clock gettime");
			return EXIT_FAILURE;
		}
		/* 1번을 누르면 */
		if (num == 1) {
			a = pthread_create(&p_thread[0], NULL, writefunc, NULL);  //쓰레드 생성
			if (a < 0) {
				perror("thread0 create error : ");
				exit(0);
			}

			b = pthread_create(&p_thread[1], NULL, data_read, NULL); //쓰레드 생성
			if (b < 0) {
				perror("thread1 create error : ");
				exit(0);
			}
			/* 스레드를 생성하고 각 스레드마다 join을 해준다 */
			pthread_join(p_thread[0], (void**)&status);
			pthread_join(p_thread[1], (void**)&status);
		}
		else if (num == 0) {//종료
			break;
		}
		//걸린시간 출력
		if (clock_gettime(CLOCK_MONOTONIC, &stop) == -1) {//시간측정종료
			perror("clock gettime");
			return EXIT_FAILURE;
		}

		accum = (stop.tv_sec - start.tv_sec) + (double)(stop.tv_nsec - start.tv_nsec) / (double)BILLION;
		printf("---------------------------------------------\n");
		printf("시간 측정: %.9f\n", accum);
	}
	sem_destroy(&sem);
}