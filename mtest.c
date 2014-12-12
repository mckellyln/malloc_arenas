// ****************************************************************************

#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <limits.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <inttypes.h>
#include <malloc.h>
#include <sys/types.h>
#include <pwd.h>

#define MALLOC malloc
#define FREE   free

// #define NTHREADS  100
#define NTHREADS  40
#define ALLOCSIZE  16384
#define STRAGGLERS  100

static uint cpus;
static uint pages;
static uint pagesize;

static uint nallocs;

static volatile int go;
static volatile int done;
static volatile int spin;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

static void **ps;  // allocations that are freed in turn by each thread
static int nps;
static void **ss;  // straggling allocations to prevent arena free
static int nss;

void
my_sleep(
    int ms
    )
{
    int rv;
    struct timespec ts;
    struct timespec rem;

    ts.tv_sec  = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000;
    for (;;) {
        rv = nanosleep(&ts, &rem);
        if (! rv) {
            break;
        }
        assert(errno == EINTR);
        ts = rem;
    }
}

void * my_thread( void *context)
{
    int i;
    int n;
    int si;
    int rv;
    void *p;

    n = (int)(intptr_t)context;

    while (! go) {
        my_sleep(100);
    }

    // first we spin to get our own arena
    while (spin) {
        p = MALLOC(ALLOCSIZE);
        assert(p);
        if (rand()%20000 == 0) {
            my_sleep(10);
        }
        FREE(p);
    }

    my_sleep(1000);

    // then one thread at a time, do our big allocs
    rv = pthread_mutex_lock(&mutex);
    assert(! rv);

    for (i = 0; i < nallocs; i++) {
        assert(i < nps);
        ps[i] = MALLOC(ALLOCSIZE);
        assert(ps[i]);
    }

    // N.B. we leave 1 of every STRAGGLERS allocations straggling
    for (i = 0; i < nallocs; i++) {
        assert(i < nps);
        if (i%STRAGGLERS == 0) {
            si = nallocs/STRAGGLERS*n + i/STRAGGLERS;
            assert(si < nss);
            ss[si] = ps[i];
        } else {
            FREE(ps[i]);
        }
    }

    done++;
    printf("%d ", done);
    fflush(stdout);

    rv = pthread_mutex_unlock(&mutex);
    assert(! rv);

    return NULL;
}

int
main(int argc, char **argv)
{
    int i;
    int rv;
    time_t n;
    time_t t;
    time_t lt;
    pthread_t thread;
    char command[128];

#if 0
#ifdef M_ARENA_MAX
    mallopt(M_ARENA_MAX, 4);
#else
#   warning M_ARENA_MAX not supported
#endif
#endif

    if (argc > 1) {
        if (! strcmp(argv[1], "-x")) {
            spin = 1;
            argc--;
            argv++;
        }
    }
    if (argc > 1) {
        printf("usage: memx2 [-x]\n");
        return 1;
    }

    cpus = sysconf(_SC_NPROCESSORS_CONF);
    pages = sysconf (_SC_PHYS_PAGES);
    pagesize = sysconf (_SC_PAGESIZE);
    printf("cpus = %d; pages = %d; pagesize = %d\n", cpus, pages, pagesize);

    nallocs = pages/10/STRAGGLERS*STRAGGLERS;

    // use same mem for any system ...
    if (nallocs > 101300)
        nallocs = 101300;

    // nallocs = pages/100/STRAGGLERS*STRAGGLERS;

    assert(! (nallocs%STRAGGLERS));

    printf("nallocs = %d\n", nallocs);

    nps = nallocs;

    printf("about to malloc %lu bytes\n", nps*sizeof(*ps) );
    ps = MALLOC(nps*sizeof(*ps));
    assert(ps);

    printf("nss = %d\n", NTHREADS*nallocs/STRAGGLERS);

    nss = NTHREADS*nallocs/STRAGGLERS;

    printf("about to malloc %lu bytes\n", nss*sizeof(*ss) );
    ss = MALLOC(nss*sizeof(*ss));
    assert(ss);

    if (pagesize != 4096) {
        printf("WARNING -- this program expects 4096 byte pagesize!\n");
    }

    printf("--- creating %d threads ---\n", NTHREADS);
    for (i = 0; i < NTHREADS; i++) {
        rv = pthread_create(&thread, NULL, my_thread, (void *)(intptr_t)i);
        assert(! rv);
        rv = pthread_detach(thread);
        assert(! rv);
    }
    go = 1;

    if (spin) {
        printf("--- allowing threads to create preferred arenas ---\n");
        my_sleep(5000);
        spin = 0;
    }

    printf("--- waiting for threads to allocate memory ---\n");
    while (done != NTHREADS) {
        my_sleep(1000);
    }
    printf("\n");

#if 0
    printf("--- malloc_stats() ---\n");
    (void)malloc_stats();
#endif

#if 0
    sprintf(command, "cat /proc/%d/status | grep -i vm", (int)getpid());
    printf("--- %s (%d) ---\n", command, (int)getpid());
    fflush(NULL);
    (void)system(command);
    fflush(NULL);
#endif

    sprintf(command, "smem | grep \"%d %s\" | grep -v grep", (int)getpid(), getlogin());
    printf("--- %s (%d) ---\n", command, (int)getpid());
    printf("  PID User     Command                         Swap      USS      PSS      RSS\n");
    fflush(NULL);
    (void)system(command);
    fflush(NULL);

    // access the stragglers
    printf("--- accessing memory ---\n");
    t = time(NULL);
    lt = t;
    for (i = 0; i < nss; i++) {
        memset(ss[i], 0, ALLOCSIZE);
        n = time(NULL);
        if (n-lt >= 60) {
            printf("%d secs... ", (int)(n-t));
            fflush(stdout);
            lt = n;
        }
    }
    if (lt != t) {
        printf("\n");
    }
    printf("--- done in %d seconds ---\n", (int)(time(NULL)-t));

    return 0;
}

