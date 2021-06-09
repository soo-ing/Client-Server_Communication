#include "share.h"

int shm1;
int* shmaddr;
int status;
pthread_t p_thread[3];

//Ŭ���̾�Ʈ���� ������ ����
void data_send() {
	for (int i = 0; i < LISTSIZ; i++) {
		shmaddr[i] = i;
	}
	printf("���� �Ϸ�\n");
}

//�����͸� ������ ���� ������
void data_read() {
	pthread_create(&p_thread[2], NULL, data_send, NULL);
	pthread_join(p_thread[2], (void**)&status);
	return;
}

void th_start() {
	if (shmaddr[0] == 1) {	//Ŭ���̾�Ʈ�� �Է��� ���� 1�� ��
		pthread_create(&p_thread[1], NULL, data_read, NULL);
		pthread_join(p_thread[1], (void**)&status);
	}
}

void main() {
	getsem();//������ �������� ����

			 //���� �޸� ���� (Ű�� 60128)
	if ((shm1 = shmget(SHMKEY, sizeof(int) * LISTSIZ, IPC_CREAT | 0666)) == -1) {
		perror("shm1 failed");
		exit(1);
	}
	//�����޸� ���̱�
	if ((shmaddr = (int*)shmat(shm1, NULL, 0)) == (void*)-1) { 
		perror("shmat failed");
		exit(1);
	}

	while (1) {
		pthread_create(&p_thread[0], NULL, th_start, NULL);
		pthread_join(p_thread[0], (void**)&status);
	}
}
