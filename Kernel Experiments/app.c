#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_COMMAND_LEN 100
#define MIN_ALLOC 1000
#define MAX_ALLOC 1000000

pid_t pid;

void exec_module();

// A dummy recursion
void dummy_rec(int x) {
    if (x == 0) {
        usleep(1000 * 1000 * 3);
        exec_module();
    }
    else { 
        dummy_rec(x-1);
    }
}

// Ackermann function
// It grows faster than an exponential function
int ack(int x, int y) {
    if (x == 0) {
        exec_module();
        return y + 1;
    }
    if (y == 0)
        return ack(x-1, 1);
    return ack(x-1, ack(x, y-1));
}

void exec_module() {
    char command[MAX_COMMAND_LEN]; 
    sprintf(command, "insmod test-1.ko processid=%d", pid);
    system(command);
    system("rmmod test-1.o");
    system("tail -13 /var/log/syslog > a.out");
    system("cat a.out");
    puts("");
}

int main(int argc, char **argv) {
    // init pid
    pid = getpid();
    printf("initial state of the virtual memory\n");
    exec_module();

    int i;
    for (i = MIN_ALLOC; i <= MAX_ALLOC; i *= 10){ 
        char *m = malloc(i*sizeof(char));
        m[i-1] = 'A';
        printf("the state of virtual memory for %d allocations from heap\n", i);
        usleep(1000 * 1000 * 3);
        exec_module();
        free(m);
    }

    for (i = MIN_ALLOC; i <= MAX_ALLOC; i *= 10) {
        printf("the state of virtual memory for %d allocations from function call stack\n", i);
        dummy_rec(i);
    }
    
    for (i = MIN_ALLOC; i <= MAX_ALLOC; i *= 10) {
        printf("the state of virtual memory for %d allocations from the stack frame of main function\n", i);
        char dummy[i];
        dummy[i-1] = 'A';
        usleep(1000 * 1000 * 3);
        exec_module(); 
    }
   
    while(1);
    return 0;
}