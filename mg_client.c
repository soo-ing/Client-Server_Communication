#include "msg.h"

void* read_data(); // �迭�� �о��
void* write_input(); // input
void recv_result(); // ������ ���� ó���� ������� �о��

struct recvbuf sndbuf; // �迭 ������ �޴� �뵵�� ����ü
struct sendbuf rcvbuf; // ��û�� ������ �뵵�� ����ü
struct resultbuf rsbuf; // ������� �޴� �뵵�� ����ü
struct msqid_ds msq_status, msq_status2; // �޽��� ť ���� ����ü

key_t key_id, key_id2, key_id3;
pthread_t recv_thread, send_thread, time_thread;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int msgtype = 1;
a = -1; 
int serv_pid, clnt_pid; // ���� pid, Ŭ���̾�Ʈ pid

int main(int argc, char** argv)
{
	int num, m;
	struct sigaction sigact;
	struct timespec start, stop;
	double accum;

	// Message queue ����
	key_id = msgget(SHMKEY1, IPC_CREAT | 0666); // recv�� queue
	key_id2 = msgget(SHMKEY2, IPC_CREAT | 0666); // send�� queue
	key_id3 = msgget(SHMKEY3, IPC_CREAT | 0666); // result recv�� queue

	if (key_id < 0 || key_id2 < 0 || key_id3 < 0) {
		perror("msgget error : ");
		return 0;
	}
	sigemptyset(&sigact.sa_mask);
	sigact.sa_flags = SA_INTERRUPT;

	if (sigaction(SIGUSR2, &sigact, NULL) < 0)
	{       // ���� pid �޾ƿ���
		perror("sigaction error : ");
		return 0;
	}

	// ���� pid �޾ƿ���
	if (msgctl(key_id, IPC_STAT, &msq_status) == -1) {
		perror("msgctl failed");
		return 0;
	}
	serv_pid = msq_status.msg_lspid;

	// Ŭ���̾�Ʈ pid �޾ƿ���
	clnt_pid = getpid();

	while (1) {
		scanf("%d", &num);

		switch (num) {

		case 1: // 1�������� �������� ������ �޾ƿ�
			m = -1;
			if (clock_gettime(CLOCK_MONOTONIC, &start) == -1) {  //�ð���������
				perror("clock gettime");
				return EXIT_FAILURE;
			}
			pthread_create(&send_thread, NULL, write_input, (void*)&m); //������ ����
			pthread_join(send_thread, NULL); //�����带 �����ϰ� �� �����帶�� join�� ���ش�

			pthread_create(&recv_thread, NULL, read_data, NULL); //������ ����
			pthread_join(recv_thread, NULL); //�����带 �����ϰ� �� �����帶�� join�� ���ش�
			break;
		default:
			printf("�߸� �Է��ϼ̽��ϴ�.\n");
			break;
		}
		if (clock_gettime(CLOCK_MONOTONIC, &stop) == -1) {  //�ð���������
			perror("clock gettime");
			return EXIT_FAILURE;
		}
		//�ɸ��ð� ���
		accum = (stop.tv_sec - start.tv_sec) + (double)(stop.tv_nsec - start.tv_nsec) / (double)BILLION;
		printf("-----------------------\n");
		printf("�����ð�: %.9f\n", accum);
	}
	return 0;
}

/* �����κ��� �����͸� �о�� */
void* read_data()
{
	if (msgrcv(key_id, (void*)&rcvbuf, sizeof(struct sendbuf), clnt_pid, 0) == -1)
	{
		perror("msgrcv error : ");
		return 0;
	}
	for (int i = 0; i < sizeof(rcvbuf.arr) / sizeof(int); i++) {
		printf("server: %d \n", rcvbuf.arr[i]); //�������� ���� ������ ���
	}
	pthread_exit(NULL);
}

void* write_input(void* data)
{
	sndbuf.msgtype = serv_pid; // ���� pid�� ������ �������� �޽����� �ޱ� ���� ����
	sndbuf.data = *((int*)data);
	sndbuf.clnt_pid = clnt_pid; // Ŭ���̾�Ʈ pid

	if (msgsnd(key_id2, (void*)&sndbuf, sizeof(struct recvbuf), IPC_NOWAIT) == -1)
	{
		perror("msgsnd error : ");
		return 0;
	}

	kill(serv_pid, SIGUSR2); // �������� signal�� ������ �������� read_input �Լ� ����

	pthread_exit(NULL);
}

/* �����κ��� ó���� ������� �޾ƿ� */
void recv_result() {
	msgrcv(key_id3, (void*)&rsbuf, sizeof(struct resultbuf), clnt_pid, 0);

	if (msgctl(key_id3, IPC_STAT, &msq_status2) == -1) {
		perror("msgctl failed");
		exit(1);
	}
}
