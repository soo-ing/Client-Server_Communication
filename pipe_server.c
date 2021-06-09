#include "pipe.h"

int fd0, fd1;
int semid;
int nread, rc;
char msg[MSG_SIZE];   //받아온 메세지
char snd[MSG_SIZE];   //보내는 메세지				  

int status; //thread_join을 위해 사용되는 변수
pthread_t p_thread[3]; //쓰레드 생성하는데 쓰임

int semid;

void data_send();

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

int makePipe() {   //파이프 생성
				   /* 기존에 named pipe가 있으면 삭제 */
	if (access(PIPENAME0, F_OK) == 0) {
		unlink(PIPENAME0);
	}
	if (access(PIPENAME1, F_OK) == 0) {
		unlink(PIPENAME1);
	}

	/* pipe 생성하기 */
	if ((rc = mkfifo(PIPENAME0, 0666)) < 0) {
		printf("fail to make named pipe\n");
		return 0;
	}
	/* pipe 생성하기 */
	if ((rc = mkfifo(PIPENAME1, 0666)) < 0) {
		printf("fail to make named pipe\n");
		return 0;
	}

}

int pipefunc() {   //파이프 오픈
				   /*fd0은 서버가 읽고 클라가 쓰는용*/
	if ((fd0 = open(PIPENAME0, O_RDONLY)) < 0) {
		printf("fail to open named pipe\n");
		return 0;
	}
	/*fd1은 서버가 쓰고 클라가 읽는용*/
	/* named pipe 열기, Read Write가능 해야 한다 */
	if ((fd1 = open(PIPENAME1, O_WRONLY)) < 0) {
		printf("fail to open named pipe\n");
		return 0;
	}
}
void writefunc(int flag) {      //파이프에 쓰는 함수
	snprintf(snd, sizeof(snd), "%d", flag);   //char 배열 형으로 변환
	if ((nread = write(fd1, snd, sizeof(snd))) < 0) {
		printf("fail to call write()\n");
	}
}

int readfunc() {   //파이프에서 읽어오는 함수
	if ((nread = read(fd0, msg, sizeof(msg))) < 0) {
		printf("fail to call read()\n");
	}
	return atoi(msg);   //int형으로 변환해서 전달
}

void data_read() {
	pthread_create(&p_thread[2], NULL, data_send, NULL); //쓰레드 생성
	pthread_join(p_thread[2], (void**)&status); //스레드를 생성하고 각 스레드마다 join을 해준다
	return;
}

void data_send() {      //데이터 전송
	for (int i = 0; i < LISTSIZ; i++) {
		writefunc(i);
	}
	printf("전송 완료\n");
}

void main() {

	int num;

	getsem();//서버가 세마포어 생성

	makePipe();   //파이프 생성
	pipefunc();   //파이프 오픈
	int pid = getpid();   //서버의 pid

	while (1) {
		num = readfunc();  //번호 읽어오기
				
		switch (num) {
		case 1:
			data_send();   //클라이언트에게 데이터 전송
			break;
		default:
			break;
		}
	}
}