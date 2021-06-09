#include "msg.h"

void* send_data(); // �������� Ŭ���̾�Ʈ�� �����͸� ����
void* read_input(); // Ŭ���̾�Ʈ�� ��û�� ����
void send_result(int msgtype); // Ŭ���̾�Ʈ���� ������� ����
void ipc_remove(); // ���ͷ�Ʈ ���� ��, ipc �޸𸮸� ����� ����
void input_read(); // recv_thread�� ����

struct sendbuf sndbuf;
struct recvbuf rcvbuf;
struct resultbuf rsbuf; // �������� ó���� ����� ������
struct msqid_ds msq_status; // �޽��� ť�� ���� ����ü
					
Node* head; // Ŭ���̾�Ʈ pid ����Ʈ�� ����Ű�� ������
void addNodeLast(Node* head, int pid); // ���Ḯ��Ʈ�� ���ο� Ŭ���̾�Ʈ�� pid�� �߰� 
Node* newNode(int pid); // ���ο� ��� ����

key_t key_id, key_id2, key_id3;
pthread_t send_thread, recv_thread, time_thread;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; // ���ؽ� �ʱ�ȭ

int msgtype = 1;
int data;
int serv_pid;

int main(void)
{
	struct sigaction sigact, inter;

	// Message queue ����
	key_id = msgget(SHMKEY1, IPC_CREAT | 0666); // send�� queue
	key_id2 = msgget(SHMKEY2, IPC_CREAT | 0666); // recv�� queue
	key_id3 = msgget(SHMKEY3, IPC_CREAT | 0666); // result send�� queue

												 // Message queue ���� Ȯ��
	if (key_id < 0 || key_id2 < 0 || key_id3 < 0) {
		perror("msgget error : ");
		return 0;
	}

	serv_pid = getpid(); // ���� pid ��

		 // ���ͷ�Ʈ �ñ׳�
	inter.sa_handler = ipc_remove;
	sigemptyset(&inter.sa_mask);
	inter.sa_flags = SA_INTERRUPT;

	// Ŭ���̾�Ʈ���Լ� signal�� ������ read_input ����
	sigact.sa_handler = input_read;
	sigemptyset(&sigact.sa_mask);
	sigact.sa_flags = SA_INTERRUPT;

	if (sigaction(SIGUSR2, &sigact, NULL) < 0 || sigaction(SIGINT, &inter, NULL) < 0)
	{
		perror("sigaction error : ");
		return 0;
	}

	// �迭 �ʱ�ȭ
	for (int i = 0; i < 50; i++) {
		sndbuf.arr[i] = 0; // 0���� �ʱ�ȭ
	}

	// Ŭ���̾�Ʈ���� ���� pid �˷��ֱ� ����
	sndbuf.msgtype = serv_pid;

	if (msgsnd(key_id, (void*)&sndbuf, sizeof(struct sendbuf), IPC_NOWAIT) == -1) {
		perror("msgsnd error : ");
		return 0;
	}
	sleep(2);
	if (msgrcv(key_id, (void*)&sndbuf, sizeof(struct sendbuf), serv_pid, 0) == -1) {
		perror("msgrcv error : ");
		return 0;
	}

	while (1) {
		head = (Node*)malloc(sizeof(Node)); // Ŭ���̾�Ʈ pid�� ��� ���Ḯ��Ʈ head
		head->pid = 0;
		head->link = NULL;
		free(head);
	}

	return 0;
}

/* Ŭ���̾�Ʈ���� �迭 ������ ������ �Լ� �����ϸ� ���ۿϷ� */
void* send_data() {

	if (msgsnd(key_id, (void*)&sndbuf, sizeof(struct sendbuf), IPC_NOWAIT) == -1)
	{
		perror("msgsnd error : ");
		return 0;
	}
	printf("���� �Ϸ�\n");
}

/* recv_thread�� �����ϴ� �Լ� */
void input_read() {
	pthread_create(&recv_thread, NULL, read_input, NULL);
	pthread_join(recv_thread, NULL);
	return;
}

/* Ŭ���̾�Ʈ�� ��û�� �ް� �̸� ó���ϴ� �Լ� */
void* read_input() {

	int clnt_pid;

	if (msgctl(key_id2, IPC_STAT, &msq_status) == -1) {
		perror("msgctl failed");
		return 0;
	}

	if (msgrcv(key_id2, (void*)&rcvbuf, sizeof(struct recvbuf), serv_pid, 0) == -1)
	{
		perror("msgrcv error : ");
		return 0;
	}

	addNodeLast(head, rcvbuf.clnt_pid); // Ŭ���̾�ƮList�� pid �߰�

	/*���ؽ� lock -> Ŭ���̾�Ʈ�� �迭�� ���� write �ϱ⶧����
	������°� �Ͼ�� �ʵ��� mutex�� ����ؾ��Ѵ�.*/
	pthread_mutex_lock(&mutex); 

	int data = rcvbuf.data; // Ŭ���̾�Ʈ�� ���� ��û ��
	int sss = rcvbuf.sss; // Ŭ���̾�Ʈ�� �����ϰ� �ִ� �迭
	sndbuf.msgtype = rcvbuf.clnt_pid; // ��û�� ���� Ŭ���̾�Ʈ�� ����

	for (int i = 0; i < LISTSIZ; i++) {
		sndbuf.arr[i] = i;
	}
	if (rcvbuf.data == -1) {
		pthread_create(&send_thread, NULL, send_data, NULL); //������ ����
		pthread_join(send_thread, NULL);
		/* �����带 �����ϰ� �� �����帶�� join�� ���ش� */
	}
	pthread_mutex_unlock(&mutex); // ���ؽ� unlock

}

/* �������� ó���� ������� ������ �Լ� */
void send_result(int msgtype) {
	rsbuf.msgtype = msgtype;

	if (msgsnd(key_id3, (void*)&rsbuf, sizeof(struct resultbuf), 0) < 0)
	{
		perror("result send error : ");
		exit(1);
	}
	// ���� Ȯ��
	if (msgctl(key_id3, IPC_STAT, &msq_status) == -1) {
		perror("msgctl failed");
		exit(1);
	}
}

/* Ŭ���̾�Ʈ pid list�� ���ο� pid �߰� */
void addNodeLast(Node* head, int pid) {
	Node* now = head;
	Node* node = newNode(pid);

	while (now->link != NULL) {
		now = now->link;
		if (now->pid == pid) { // �̹� �����ϴ� pid�� return
			return;
		}
	}
	if (head->link == NULL) // �����ϴ� pid�� ������
		head->link = node;
	else
		now->link = node;
}

/* ���ο� ��� ���� */
Node* newNode(int pid) {
	Node* node = (Node*)malloc(sizeof(Node));
	node->pid = pid;
	node->link = NULL;
}

/* ctrl-c ������ �޽��� ť ���� */
void ipc_remove() {
	if (msgctl(key_id, IPC_RMID, 0) < 0 || msgctl(key_id2, IPC_RMID, 0) < 0 || msgctl(key_id3, IPC_RMID, 0) < 0) {
		perror("msg queue remove fail : ");
		exit(1);
	}
	printf("ť�� ���ŵǾ����ϴ�.\n");
	exit(1);
}

