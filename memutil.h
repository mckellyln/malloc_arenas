#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <limits.h>
#include <pthread.h>
#include <inttypes.h>

extern uint64_t getHeapInfo(void);
extern uint64_t getVMSize(const char *type);
extern uint64_t getMallocInfo(void);

