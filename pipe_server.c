#include "pipe.h"

int fd0, fd1;
int semid;
int nread, rc;
char msg[MSG_SIZE];   //�޾ƿ� �޼���
char snd[MSG_SIZE];   //������ �޼���				  

int status; //thread_join�� ���� ���Ǵ� ����
pthread_t p_thread[3]; //������ �����ϴµ� ����

int semid;

void data_send();

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

int makePipe() {   //������ ����
				   /* ������ named pipe�� ������ ���� */
	if (access(PIPENAME0, F_OK) == 0) {
		unlink(PIPENAME0);
	}
	if (access(PIPENAME1, F_OK) == 0) {
		unlink(PIPENAME1);
	}

	/* pipe �����ϱ� */
	if ((rc = mkfifo(PIPENAME0, 0666)) < 0) {
		printf("fail to make named pipe\n");
		return 0;
	}
	/* pipe �����ϱ� */
	if ((rc = mkfifo(PIPENAME1, 0666)) < 0) {
		printf("fail to make named pipe\n");
		return 0;
	}

}

int pipefunc() {   //������ ����
				   /*fd0�� ������ �а� Ŭ�� ���¿�*/
	if ((fd0 = open(PIPENAME0, O_RDONLY)) < 0) {
		printf("fail to open named pipe\n");
		return 0;
	}
	/*fd1�� ������ ���� Ŭ�� �д¿�*/
	/* named pipe ����, Read Write���� �ؾ� �Ѵ� */
	if ((fd1 = open(PIPENAME1, O_WRONLY)) < 0) {
		printf("fail to open named pipe\n");
		return 0;
	}
}
void writefunc(int flag) {      //�������� ���� �Լ�
	snprintf(snd, sizeof(snd), "%d", flag);   //char �迭 ������ ��ȯ
	if ((nread = write(fd1, snd, sizeof(snd))) < 0) {
		printf("fail to call write()\n");
	}
}

int readfunc() {   //���������� �о���� �Լ�
	if ((nread = read(fd0, msg, sizeof(msg))) < 0) {
		printf("fail to call read()\n");
	}
	return atoi(msg);   //int������ ��ȯ�ؼ� ����
}

void data_read() {
	pthread_create(&p_thread[2], NULL, data_send, NULL); //������ ����
	pthread_join(p_thread[2], (void**)&status); //�����带 �����ϰ� �� �����帶�� join�� ���ش�
	return;
}

void data_send() {      //������ ����
	for (int i = 0; i < LISTSIZ; i++) {
		writefunc(i);
	}
	printf("���� �Ϸ�\n");
}

void main() {

	int num;

	getsem();//������ �������� ����

	makePipe();   //������ ����
	pipefunc();   //������ ����
	int pid = getpid();   //������ pid

	while (1) {
		num = readfunc();  //��ȣ �о����
				
		switch (num) {
		case 1:
			data_send();   //Ŭ���̾�Ʈ���� ������ ����
			break;
		default:
			break;
		}
	}
}