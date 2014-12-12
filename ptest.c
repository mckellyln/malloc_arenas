#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/time.h>
#include <pthread.h>

#define FALSE 0
#define TRUE 1

pid_t gettid(void);
void *membench(void *arg);


// Argument for threads
typedef struct {
  int mallocSize;
  int count;
  int loop;
  int verbose;
} THREAD_ARGS;


// main
int main(int argc, char **argv){

  int c;
  int threadNum = 1;

  THREAD_ARGS thread_args;
  thread_args.mallocSize = 1;
  thread_args.count = 1;
  thread_args.loop = 1;
  thread_args.verbose = FALSE;

  static struct option long_options[] =
  {
    {"threads", required_argument, 0, 't'},
    {"size", required_argument, 0, 's'},
    {"count", required_argument, 0, 'c'},
    {"loop", required_argument, 0, 'l'},
    {"verbose", no_argument, 0, 'v'},
    {0, 0, 0, 0}
  };

  int option_index = 0;

  while((c = getopt_long( argc, argv, "t:s:c:l:v", long_options, &option_index)) != -1) {

    switch(c) {
      case 't':
        threadNum = atoi(optarg);
        break;
      case 's':
        thread_args.mallocSize = atoi(optarg);
        break;
      case 'c':
        thread_args.count = atoi(optarg);
        break;
      case 'l':
        thread_args.loop = atoi(optarg);
        break;
      case 'v':
        thread_args.verbose = TRUE;
        break;
      case ':':
      case '?':
        fprintf(stderr, "Usage: %s --thread=THREADNUMBER --size=MALLOCSIZE --count=MALLOCCOUNT --loop=LOOPCOUNT\n", argv[0]);
        exit(EXIT_FAILURE);
    }
  }

  int i;
  pthread_t p[threadNum];
  struct timeval t0, t1;

  printf("Number of threads                      : %d\n", threadNum);
  printf("Malloc size                            : %d MB\n", thread_args.mallocSize);
  printf("Number of councurrent malloc per thread: %d\n", thread_args.count);
  printf("Number of loops per thread             : %d\n", thread_args.loop);

  // Get Starttime
  gettimeofday(&t0, NULL);

  // Create Threads
  for( i = 0; i < threadNum; i++) {
    pthread_create(&p[i], NULL, membench, &thread_args);
  }

  // Closed Threads
  for( i = 0; i < threadNum; i++) {
    pthread_join(p[i], NULL);
  }

  // Get Endtime 
  gettimeofday(&t1, NULL);

  char command[256];
  sprintf(command, "cat /proc/%d/status | grep -i vm", (int)getpid());
  printf("--- %s ---\n", command);
  (void)system(command);

  // Get Elapsed time
  t1.tv_sec -= t0.tv_sec;
  if (t1.tv_usec < t0.tv_usec) {
    t1.tv_sec -=1;
    t1.tv_usec += 1000000 - t0.tv_usec;
  } else {
    t1.tv_usec -= t0.tv_usec;
  }

  printf("%d.%06d sec\n", (int)t1.tv_sec, (int)t1.tv_usec);

  exit(EXIT_SUCCESS);
}

// Get Thread Id
pid_t gettid(void) {
  return syscall(SYS_gettid);
}

// Memory Benchmark
void *membench(void *arg) {

  THREAD_ARGS *thread_args = (THREAD_ARGS*)arg;

  int mallocSize = thread_args->mallocSize;
  int count      = thread_args->count;
  int loop       = thread_args->loop;
  int verbose    = thread_args->verbose;

  int *mem[count];
  int size[count];

  int i, loopcnt = 0;

  while (1) {

    if ( loop != 0 && ++loopcnt > loop) {
      break;
    }

    // Malloc()
    for(i = 0; i < count; i++) {
      size[i] = ( rand() % (mallocSize * 10)  + 1 ) * 100 * 1000;
      mem[i]  = (int *)malloc( size[i] );

      if (!mem[i]) {
        puts("can't malloc");
        return NULL;
      }
      memset(mem[i], 1, size[i]);
      memset(mem[i], 0, size[i]);
      if (verbose) {
        printf("thread: %d, loop: %d, mem[%d] %d Bytes malloc\n", gettid(), loopcnt, i, size[i]);
      }
    }

    // Free()
    for(i = 0; i < count; i++) {
      free(mem[i]);
      if (verbose) {
        printf("thread: %d, loop: %d, mem[%d] %d Bytes free\n", gettid(), loopcnt, i, size[i]);
      }
    }
  }
  return NULL;
}

