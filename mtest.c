// ****************************************************************************

#ifdef JEMALLOC
# include <jemalloc/jemalloc.h>
#endif

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

#include "memutil.h"

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

static volatile int go = 0;
static volatile int done = 0;
static volatile int spin = 0;
static volatile int dosleep = 0;
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

#if 0
#ifdef M_ARENA_MAX
    mallopt(M_ARENA_MAX, 4);
#else
#   warning M_ARENA_MAX not supported
#endif
#endif

#ifdef JEMALLOC
    size_t version;
    size_t vlen = sizeof(size_t);
    mallctl("version", &version, &vlen, NULL, 0);
    printf("jemalloc version: %s\n", (char *)version);
#endif

   int opt;
   while ((opt = getopt(argc, argv, "xs")) != -1) {
        switch (opt) {
            case 's':   dosleep = 1; break;
            case 'x':   spin = 1; break;
            default:
                printf("usage: %s [-x] [-s]\n", argv[0]);
                return 1;
                break;
        }
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

    printf("getHeapInfo(): %lu\n", getHeapInfo());
    printf("getVmSize(\"VmPeak\"): %lu\n", getVMSize("VmPeak"));
    printf("getVmSize(\"VmSize\"): %lu\n", getVMSize("VmSize"));
    printf("getVmSize(\"VmData\"): %lu\n", getVMSize("VmData"));

    printf("about to malloc %lu bytes\n", nps*sizeof(*ps) );
    ps = MALLOC(nps*sizeof(*ps));
    assert(ps);

    printf("nss = %d\n", NTHREADS*nallocs/STRAGGLERS);

    nss = NTHREADS*nallocs/STRAGGLERS;

    printf("about to malloc %lu bytes\n", nss*sizeof(*ss) );
    ss = MALLOC(nss*sizeof(*ss));
    assert(ss);

#if 0
    char *p = alloca(10000000);
    if (!p)
        printf("alloca failed, errno = %d\n", errno);
    else
        printf("alloca() ok ...\n");
#endif

    printf("getHeapInfo(): %lu\n", getHeapInfo());
    printf("getVmSize(\"VmPeak\"): %lu\n", getVMSize("VmPeak"));
    printf("getVmSize(\"VmSize\"): %lu\n", getVMSize("VmSize"));
    printf("getVmSize(\"VmData\"): %lu\n", getVMSize("VmData"));

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
    char command[128];
    sprintf(command, "cat /proc/%d/status | grep -i vm", (int)getpid());
    printf("--- %s (%d) ---\n", command, (int)getpid());
    fflush(NULL);
    (void)system(command);
    fflush(NULL);

    sprintf(command, "smem -U %s | grep \"%d \" | grep -v grep", getlogin(), (int)getpid());
    printf("--- %s ---\n", command);
    printf("  PID User     Command                         Swap      USS      PSS      RSS\n");
    fflush(NULL);
    char line[1001];
    FILE *fp = popen(command, "r");
    if (fp != NULL) {
        while(fgets(line, 100, fp) != NULL) {
            fprintf(stdout, "%s", line);
        }
        pclose(fp);
    } else {
        fprintf(stdout,"Error, unable to popen(), errno = %d\n", errno);
    }
    // (void)system(command);
    fflush(NULL);
#endif

    // sleep(100);

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

    printf("getHeapInfo(): %lu\n", getHeapInfo());
    printf("getVmSize(\"VmPeak\"): %lu\n", getVMSize("VmPeak"));
    printf("getVmSize(\"VmSize\"): %lu\n", getVMSize("VmSize"));
    printf("getVmSize(\"VmData\"): %lu\n", getVMSize("VmData"));

#ifdef JEMALLOC
    // Update the statistics cached by mallctl.
    uint64_t epoch = 1;
    size_t sz = sizeof(epoch);
    mallctl("epoch", &epoch, &sz, &epoch, sz);

    // Get basic allocation statistics.  Take care to check for
    // errors, since --enable-stats must have been specified at
    // build time for these statistics to be available.
    size_t allocated, active, mapped;
    sz = sizeof(size_t);
    if (mallctl("stats.allocated", &allocated, &sz, NULL, 0) == 0
            && mallctl("stats.active", &active, &sz, NULL, 0) == 0
            && mallctl("stats.mapped", &mapped, &sz, NULL, 0) == 0) {
        fprintf(stderr, "Current allocated/active/mapped: %zu/%zu/%zu\n", allocated, active, mapped);
    }
#endif

    fflush(NULL);
    if (dosleep)
        sleep(100);
    return 0;
}

