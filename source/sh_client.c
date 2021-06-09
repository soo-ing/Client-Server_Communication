#include "share.h"

pthread_t p_thread[3];
int shm1;
int* shmaddr;
int th, th1, th2, status;
sem_t sem;
struct timespec start, stop;
double accum;

//서버로 데이터 전송
void data_send() {
	sem_wait(&sem);	//send할 때는 recv를 못함
	if (clock_gettime(CLOCK_MONOTONIC, &start) == -1) {//시간측정시작
		perror("clock gettime");
		return EXIT_FAILURE;
	}
	shmaddr[0] = 1;
	sem_post(&sem);
}

//서버로부터 데이터를 읽어옴
void data_read() {
	sem_wait(&sem);	//recv할 때는 send를 못함
	for (int i = 0; i < LISTSIZ; i++) {
		printf("server: %d\n", shmaddr[i]);
	}
	if (clock_gettime(CLOCK_MONOTONIC, &stop) == -1) {//시간측정종료
		perror("clock gettime");
		return EXIT_FAILURE;
	}
	//걸린시간 출력
	accum = (stop.tv_sec - start.tv_sec) + (double)(stop.tv_nsec - start.tv_nsec) / (double)BILLION;
	printf("%.9f\n", accum);

	sem_post(&sem);
}

void thread_start() {
	pthread_create(&p_thread[1], NULL, data_send, NULL);	//데이터 보내기
	pthread_create(&p_thread[2], NULL, data_read, NULL);		//데이터 받기
	pthread_join(p_thread[1], (void**)&status);
	pthread_join(p_thread[2], (void**)&status);
}

void main(void) {
	int com = 0;
	//공유 메모리 생성 (키는 60128)
	if ((shm1 = shmget(SHMKEY, sizeof(int) * LISTSIZ, IPC_CREAT | 0666)) == -1) {
		perror("shm1 failed");
		exit(1);
	}
	//공유메모리 붙이기
	if ((shmaddr = shmat(shm1, 0, 0)) == (void*)-1) {
		perror("shmat failed");
		exit(1);
	}

	sem_init(&sem, 0, 1);	//세마포어 초기화
	while (1) {
		scanf("%d", &com);
		if (com == 1) {
			th = pthread_create(&p_thread[0], NULL, thread_start, NULL);
			if (th < 0) {
				perror("thread create error : ");
				exit(0);
			}
			pthread_join(p_thread[0], (void**)&status);
		}
		else if (com == 0)  break;
	}
	sem_destroy(&sem);	//세마포어 삭제
}
