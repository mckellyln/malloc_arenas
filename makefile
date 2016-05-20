
all:	mtestdef mtesttc mtestje mtestdl mtesttbb t-test1def t-test1tc t-test1je t-test1dl t-test1tbb

memutil.o: memutil.c
        gcc -O2 -g -Wall -std=gnu99 $< -c -o $@

mtestdef: mtest.c memutil.o makefile
	    gcc -O2 -g -Wall -std=gnu99 -Wno-unused-result -DDEFMALLOC mtest.c memutil.o -o $@ -lpthread

mtesttc: mtest.c memutil.o makefile
	    gcc -O2 -g -Wall -std=gnu99 -Wno-unused-result -DTCMALLOC mtest.c memutil.o -o $@ -Wl,-rpath,/home/mkelly/tcmalloc/lib -L/home/mkelly/tcmalloc/lib -ltcmalloc_minimal -lpthread

mtestje: mtest.c memutil.o makefile
	    gcc -O2 -g -Wall -std=gnu99 -Wno-unused-result -DJEMALLOC mtest.c memutil.o -o $@ -I/home/mkelly/jemalloc/include -Wl,-rpath,/home/mkelly/jemalloc/lib -L/home/mkelly/jemalloc/lib -ljemalloc -lpthread -lrt

mtestdl: mtest.c malloc.c memutil.o makefile
	    gcc -O2 -g -Wall -std=gnu99 -Wno-unused-result -DDLMALLOC mtest.c malloc.c memutil.o -o $@ -lpthread

mtesttbb: mtest.c malloc.c memutil.o makefile
	    gcc -O2 -g -Wall -std=gnu99 -Wno-unused-result -DTBBMALLOC mtest.c malloc.c memutil.o -o $@ -lpthread -ltbbmalloc_proxy

#
# NOTE for ptmalloc make sure to build it with linux-pthread or it will not be thread-safe
#
mtestpt3: mtest.c memutil.o makefile
	    gcc -O2 -g -Wall -std=gnu99 -Wno-unused-result mtest.c memutil.o -o $@ -L/home/mkelly/Downloads/ptmalloc3 -lptmalloc3 -lpthread

mtestll: mtest.c memutil.o makefile
	    gcc -O2 -g -Wall -std=gnu99 -Wno-unused-result mtest.c memutil.o -o $@ -L/home/mkelly/Downloads/lockless_allocator -lllalloc -lpthread

mtesthoard: mtest.c memutil.o makefile
	    gcc -O2 -g -Wall -std=gnu99 -Wno-unused-result mtest.c memutil.o -o $@ -Wl,-rpath,/home/mkelly/Downloads/Hoard/src -L/home/mkelly/Downloads/Hoard/src -lhoard -lpthread

mtestlockless: mtest.c memutil.o makefile
	    gcc -O2 -g -Wall -std=gnu99 -Wno-unused-result mtest.c memutil.o -o $@ -L/home/mkelly/Downloads/lockless_allocator -lllalloc -lpthread

mtestgc: mtest.c memutil.o makefile
	    gcc -O2 -g -Wall -std=gnu99 -Wno-unused-result mtest.c -D_USE_GC -I/home/mkelly/GCmalloc/include memutil.o -o $@ -Wl,-rpath,/home/mkelly/GCmalloc/lib -L/home/mkelly/GCmalloc/lib -lgc -lpthread

ptestdef: ptest.c memutil.o makefile
	    gcc -O2 -g -Wall -std=gnu99 -Wno-unused-result ptest.c memutil.o -o $@ -lpthread

ptesttc: ptest.c memutil.o makefile
	    gcc -O2 -g -Wall -std=gnu99 -Wno-unused-result ptest.c memutil.o -o $@ -Wl,-rpath,/home/mkelly/tcmalloc/lib -L/home/mkelly/tcmalloc/lib -ltcmalloc_minimal -lpthread

ptestje: ptest.c memutil.o makefile
	    gcc -O2 -g -Wall -std=gnu99 -Wno-unused-result ptest.c memutil.o -o $@ -Wl,-rpath,/home/mkelly/jemalloc/lib -L/home/mkelly/jemalloc/lib -ljemalloc -lpthread -lrt

ptestgc: ptest.c memutil.o makefile
	    gcc -O2 -g -Wall -std=gnu99 -Wno-unused-result ptest.c -D_USE_GC -I/home/mkelly/GCmalloc/include memutil.o -o $@ -Wl,-rpath,/home/mkelly/GCmalloc/lib -L/home/mkelly/GCmalloc/lib -lgc -lpthread

t-test1def: t-test1.c memutil.o makefile
	    gcc -O2 -g -Wall -std=gnu99 -Wno-unused-result -DDEFMALLOC t-test1.c memutil.o -o $@ -lpthread

t-test1tc: t-test1.c memutil.o makefile
	    gcc -O2 -g -Wall -std=gnu99 -Wno-unused-result -DTCMALLOC t-test1.c memutil.o -o $@ -Wl,-rpath,/home/mkelly/tcmalloc/lib -L/home/mkelly/tcmalloc/lib -ltcmalloc_minimal -lpthread

t-test1je: t-test1.c memutil.o makefile
	    gcc -O2 -g -Wall -std=gnu99 -Wno-unused-result -DJEMALLOC t-test1.c memutil.o -o $@ -I/home/mkelly/jemalloc/include -Wl,-rpath,/home/mkelly/jemalloc/lib -L/home/mkelly/jemalloc/lib -ljemalloc -lpthread -lrt

t-test1dl: t-test1.c malloc.c memutil.o makefile
	    gcc -O2 -g -Wall -std=gnu99 -Wno-unused-result -DDLMALLOC t-test1.c malloc.c memutil.o -o $@ -lpthread

t-test1tbb: t-test1.c malloc.c memutil.o makefile
	    gcc -O2 -g -Wall -std=gnu99 -Wno-unused-result -DTBBMALLOC t-test1.c malloc.c memutil.o -o $@ -lpthread -ltbbmalloc_proxy

#
# NOTE for ptmalloc make sure to build it with linux-pthread or it will not be thread-safe
#
t-test1pt3: t-test1.c memutil.o makefile
	    gcc -O2 -g -Wall -std=gnu99 -Wno-unused-result t-test1.c memutil.o -o $@ -L/home/mkelly/Downloads/ptmalloc3 -lptmalloc3 -lpthread

t-test1ll: t-test1.c memutil.o makefile
	    gcc -O2 -g -Wall -std=gnu99 -Wno-unused-result t-test1.c memutil.o -o $@ -L/home/mkelly/Downloads/lockless_allocator -lllalloc -lpthread

clean:
	    $(RM) -f mtestdef mtesttc mtestje mtestdl mtesttbb mtestpt3 mtestll mtesthoard mtestlockless ptestdef ptesttc ptestje mtestgc ptestgc t-test1def t-test1tc t-test1je t-test1dl t-test1pt3 t-test1ll t-test1tbb memutil.o

