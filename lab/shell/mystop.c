/* 
 * mystop.c - Another handy routine for testing your tiny shell
 * 
 * usage: mystop <n>
 * Sleeps for <n> seconds and sends SIGTSTP to itself.
 *
 */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>

long long m_start;
long long timestamp() {
    struct timeval tp;
    gettimeofday(&tp, NULL);
    return ((long long) (tp.tv_sec)) * 1000000 + tp.tv_usec;
}
void restart() { m_start = timestamp(); }
long long elapsed() { return timestamp() - m_start; }


int main(int argc, char **argv) {
    int i, secs;
    pid_t pid;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <n>\n", argv[0]);
        exit(0);
    }
    secs = atoi(argv[1]);

    restart();

    for (i = 0; i < secs; i++)
        sleep(1);

    printf("time: %lld(us)\n", elapsed());

    pid = getpid();

    if (kill(-pid, SIGTSTP) < 0)
        fprintf(stderr, "kill (tstp) error\n");

    exit(0);

}