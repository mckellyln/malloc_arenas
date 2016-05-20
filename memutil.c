#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <limits.h>
#include <pthread.h>
#include <malloc.h>

uint64_t getHeapInfo(void)
{
    uint64_t ret = 0;
    char procMaps[256];
    sprintf(procMaps, "/proc/self/maps");
    FILE *diskfp = fopen(procMaps, "r");
    if (!diskfp)
        return 0;
    char ln[256];
    while (fgets(ln, sizeof(ln), diskfp))
    {
        if ( strstr(ln, "[heap]") || (!strstr(ln, " /") && !strstr(ln, " [")) )
        {
            if ( strstr(ln, " rw-p") || strstr(ln, " ---p") )
            {
                uint64_t addrLow, addrHigh;
                if (2 == sscanf(ln, "%16lx-%16lx", &addrLow, &addrHigh))
                {
                    uint64_t ret2 = (uint64_t)(addrHigh-addrLow) / 1024ULL;
                    ret += ret2;
                }
            }
        }
    }
    fclose(diskfp);
    return ret;
}

uint64_t getVMSize(const char *type)
{
    char name[256];
    sprintf(name, "%s:", type);
    int len = (int)strlen(name);
    uint64_t ret = 0;
    char procMaps[256];
    sprintf(procMaps, "/proc/self/status");
    FILE *diskfp = fopen(procMaps, "r");
    if (!diskfp)
        return 0;
    char ln[256];
    uint64_t dataSize = 0;
    char unitstr[256];
    while (fgets(ln, sizeof(ln), diskfp))
    {
        if (!strncmp(ln, name, len))
        {
            if (2 == sscanf(&ln[7], "%lu%s", &dataSize, unitstr))
            {
                if (!strcasecmp(unitstr, "mB"))
                    dataSize *= 1024ULL;
                if (!strcasecmp(unitstr, "gB"))
                    dataSize *= 1024ULL * 1024ULL;
                ret = dataSize;
            }
        }
    }
    fclose(diskfp);
    return ret;
}

uint64_t getMallocInfo(void)
{
    char *ptr = NULL;
    size_t size = 0;
    unsigned num_arenas = 0;
    FILE *fp = open_memstream(&ptr, &size);
    if (fp)
    {
        malloc_info(0, fp);
        char line[1001] = { "" };
        char last_line[1001] = { "" };
        while(fgets(line, 1000, fp))
        {
            // fprintf(stderr,"line = %s", line);
            char *sptr = NULL;
            if ((sptr = strstr(line, "system type=\"current\" size=")))
            {
                num_arenas++;
                strcpy(last_line, line);
            }
        }
        fclose(fp);
        if (ptr)
            free(ptr);
        if (num_arenas)
        {
            num_arenas--;
            fprintf(stderr, "num_arenas = %d size = %s", num_arenas, last_line);
            char *sptr = strtok(last_line, "=");
            if (sptr != NULL)
            {
                sptr = strtok(NULL, "=");
                if (sptr != NULL)
                {
                    sptr = strtok(NULL, "=");
                    if (sptr != NULL)
                    {
                        // format is now: "value"/
                        uint64_t value;
                        if (1 == sscanf(sptr, "\"%lu\"/", &value))
                            return value;
                    }
                }
            }
        }
    }
    return 0;
}

