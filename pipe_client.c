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

/* �������� ����*/
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

int pipefunc() {   //pipe �о���� �Լ�
				   /*fd0�� ������ �а� Ŭ�� ���¿�*/
	if ((fd0 = open(PIPENAME0, O_WRONLY)) < 0) {
		printf("fail to open named pipe0\n");
		return 0;
	}
	/* pipe ����, Read Write���� �ؾ� �Ѵ� */
	/*fd1�� ������ ���� Ŭ�� �д¿�*/
	if ((fd1 = open(PIPENAME1, O_RDONLY)) < 0) {
		printf("fail to open named pipe1\n");
		return 0;
	}
}



void *writefunc(void *flag) {   //pipe�� write�ϴ� �Լ�
	sem_wait(&sem);
	snprintf(snd, sizeof(snd), "%d", 1);
	if ((nread = write(fd0, snd, sizeof(snd))) < 0) {
		printf("fail to call write()\n");
	}

	sem_post(&sem);
}

void *data_read(void *arg) {   //������ �о����						
	sem_wait(&sem);

	for (i = 0; i < LISTSIZ; i++) {
		if ((nread = read(fd1, msg, sizeof(msg))) < 0) {
			printf("read failed\n");
		}
		printf("server: %s\n", msg);   //�������� ���� ������ ���
	}
	sem_post(&sem);
}

void main(void) {
	pipefunc();   //������ ����
	int status;
	int num= 0;
	sem_init(&sem, 0, 1);

	struct timespec start, stop;
	double accum;

	while (1) {
		scanf("%d", &num);
		if (clock_gettime(CLOCK_MONOTONIC, &start) == -1) {   //�ð���������
			perror("clock gettime");
			return EXIT_FAILURE;
		}
		/* 1���� ������ */
		if (num == 1) {
			a = pthread_create(&p_thread[0], NULL, writefunc, NULL);  //������ ����
			if (a < 0) {
				perror("thread0 create error : ");
				exit(0);
			}

			b = pthread_create(&p_thread[1], NULL, data_read, NULL); //������ ����
			if (b < 0) {
				perror("thread1 create error : ");
				exit(0);
			}
			/* �����带 �����ϰ� �� �����帶�� join�� ���ش� */
			pthread_join(p_thread[0], (void**)&status);
			pthread_join(p_thread[1], (void**)&status);
		}
		else if (num == 0) {//����
			break;
		}
		//�ɸ��ð� ���
		if (clock_gettime(CLOCK_MONOTONIC, &stop) == -1) {//�ð���������
			perror("clock gettime");
			return EXIT_FAILURE;
		}

		accum = (stop.tv_sec - start.tv_sec) + (double)(stop.tv_nsec - start.tv_nsec) / (double)BILLION;
		printf("---------------------------------------------\n");
		printf("�ð� ����: %.9f\n", accum);
	}
	sem_destroy(&sem);
}