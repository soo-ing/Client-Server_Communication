#include "msg.h"

void* read_data(); // 배열를 읽어옴
void* write_input(); // input
void recv_result(); // 서버로 부터 처리된 결과값을 읽어옴

struct recvbuf sndbuf; // 배열 정보를 받는 용도의 구조체
struct sendbuf rcvbuf; // 요청을 보내는 용도의 구조체
struct resultbuf rsbuf; // 결과값을 받는 용도의 구조체
struct msqid_ds msq_status, msq_status2; // 메시지 큐 상태 구조체

key_t key_id, key_id2, key_id3;
pthread_t recv_thread, send_thread, time_thread;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int msgtype = 1;
a = -1; 
int serv_pid, clnt_pid; // 서버 pid, 클라이언트 pid

int main(int argc, char** argv)
{
	int num, m;
	struct sigaction sigact;
	struct timespec start, stop;
	double accum;

	// Message queue 생성
	key_id = msgget(SHMKEY1, IPC_CREAT | 0666); // recv용 queue
	key_id2 = msgget(SHMKEY2, IPC_CREAT | 0666); // send용 queue
	key_id3 = msgget(SHMKEY3, IPC_CREAT | 0666); // result recv용 queue

	if (key_id < 0 || key_id2 < 0 || key_id3 < 0) {
		perror("msgget error : ");
		return 0;
	}
	sigemptyset(&sigact.sa_mask);
	sigact.sa_flags = SA_INTERRUPT;

	if (sigaction(SIGUSR2, &sigact, NULL) < 0)
	{       // 서버 pid 받아오기
		perror("sigaction error : ");
		return 0;
	}

	// 서버 pid 받아오기
	if (msgctl(key_id, IPC_STAT, &msq_status) == -1) {
		perror("msgctl failed");
		return 0;
	}
	serv_pid = msq_status.msg_lspid;

	// 클라이언트 pid 받아오기
	clnt_pid = getpid();

	while (1) {
		scanf("%d", &num);

		switch (num) {

		case 1: // 1번누르면 서버에서 데이터 받아옴
			m = -1;
			if (clock_gettime(CLOCK_MONOTONIC, &start) == -1) {  //시간측정시작
				perror("clock gettime");
				return EXIT_FAILURE;
			}
			pthread_create(&send_thread, NULL, write_input, (void*)&m); //쓰레드 생성
			pthread_join(send_thread, NULL); //스레드를 생성하고 각 스레드마다 join을 해준다

			pthread_create(&recv_thread, NULL, read_data, NULL); //쓰레드 생성
			pthread_join(recv_thread, NULL); //스레드를 생성하고 각 스레드마다 join을 해준다
			break;
		default:
			printf("잘못 입력하셨습니다.\n");
			break;
		}
		if (clock_gettime(CLOCK_MONOTONIC, &stop) == -1) {  //시간측정시작
			perror("clock gettime");
			return EXIT_FAILURE;
		}
		//걸린시간 출력
		accum = (stop.tv_sec - start.tv_sec) + (double)(stop.tv_nsec - start.tv_nsec) / (double)BILLION;
		printf("-----------------------\n");
		printf("측정시간: %.9f\n", accum);
	}
	return 0;
}

/* 서버로부터 데이터를 읽어옴 */
void* read_data()
{
	if (msgrcv(key_id, (void*)&rcvbuf, sizeof(struct sendbuf), clnt_pid, 0) == -1)
	{
		perror("msgrcv error : ");
		return 0;
	}
	for (int i = 0; i < sizeof(rcvbuf.arr) / sizeof(int); i++) {
		printf("server: %d \n", rcvbuf.arr[i]); //서버에서 받은 데이터 출력
	}
	pthread_exit(NULL);
}

void* write_input(void* data)
{
	sndbuf.msgtype = serv_pid; // 서버 pid를 가지고 서버에서 메시지를 받기 위해 설정
	sndbuf.data = *((int*)data);
	sndbuf.clnt_pid = clnt_pid; // 클라이언트 pid

	if (msgsnd(key_id2, (void*)&sndbuf, sizeof(struct recvbuf), IPC_NOWAIT) == -1)
	{
		perror("msgsnd error : ");
		return 0;
	}

	kill(serv_pid, SIGUSR2); // 서버에게 signal을 보내면 서버에서 read_input 함수 시행

	pthread_exit(NULL);
}

/* 서버로부터 처리된 결과값을 받아옴 */
void recv_result() {
	msgrcv(key_id3, (void*)&rsbuf, sizeof(struct resultbuf), clnt_pid, 0);

	if (msgctl(key_id3, IPC_STAT, &msq_status2) == -1) {
		perror("msgctl failed");
		exit(1);
	}
}
