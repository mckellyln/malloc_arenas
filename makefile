
all:	mtestdef mtesttc mtestje mtestdl t-test1def t-test1tc t-test1je t-test1dl

mtestdef: mtest.c makefile
	    gcc -O2 -Wall -std=gnu99 -Wno-unused-result mtest.c -o $@ -lpthread

mtesttc: mtest.c makefile
	    gcc -O2 -Wall -std=gnu99 -Wno-unused-result mtest.c -o $@ -Wl,-rpath,/home/mkelly/tcmalloc/lib -L/home/mkelly/tcmalloc/lib -ltcmalloc_minimal -lpthread

mtestje: mtest.c makefile
	    gcc -O2 -Wall -std=gnu99 -Wno-unused-result mtest.c -o $@ -Wl,-rpath,/home/mkelly/jemalloc/lib -L/home/mkelly/jemalloc/lib -ljemalloc -lpthread

mtestdl: mtest.c malloc.c makefile
	    gcc -O2 -Wall -std=gnu99 -Wno-unused-result mtest.c malloc.c -o $@ -lpthread

mtestpt3: mtest.c makefile
	    gcc -O2 -Wall -std=gnu99 -Wno-unused-result mtest.c -o $@ -L/home/mkelly/Downloads/ptmalloc3 -lptmalloc3 -lpthread

mtestll: mtest.c makefile
	    gcc -O2 -Wall -std=gnu99 -Wno-unused-result mtest.c -o $@ -L/home/mkelly/Downloads/lockless_allocator -lllalloc -lpthread

mtesthoard: mtest.c makefile
	    gcc -O2 -Wall -std=gnu99 -Wno-unused-result mtest.c -o $@ -Wl,-rpath,/home/mkelly/Downloads/Hoard/src -L/home/mkelly/Downloads/Hoard/src -lhoard -lpthread

mtestlockless: mtest.c makefile
	    gcc -O2 -Wall -std=gnu99 -Wno-unused-result mtest.c -o $@ -L/home/mkelly/Downloads/lockless_allocator -lllalloc -lpthread

mtestgc: mtest.c makefile
	    gcc -O2 -Wall -std=gnu99 -Wno-unused-result mtest.c -D_USE_GC -I/home/mkelly/GCmalloc/include -o $@ -Wl,-rpath,/home/mkelly/GCmalloc/lib -L/home/mkelly/GCmalloc/lib -lgc -lpthread

ptestdef: ptest.c makefile
	    gcc -O2 -Wall -std=gnu99 -Wno-unused-result ptest.c -o $@ -lpthread

ptesttc: ptest.c makefile
	    gcc -O2 -Wall -std=gnu99 -Wno-unused-result ptest.c -o $@ -Wl,-rpath,/home/mkelly/tcmalloc/lib -L/home/mkelly/tcmalloc/lib -ltcmalloc_minimal -lpthread

ptestje: ptest.c makefile
	    gcc -O2 -Wall -std=gnu99 -Wno-unused-result ptest.c -o $@ -Wl,-rpath,/home/mkelly/jemalloc/lib -L/home/mkelly/jemalloc/lib -ljemalloc -lpthread

ptestgc: ptest.c makefile
	    gcc -O2 -Wall -std=gnu99 -Wno-unused-result ptest.c -D_USE_GC -I/home/mkelly/GCmalloc/include -o $@ -Wl,-rpath,/home/mkelly/GCmalloc/lib -L/home/mkelly/GCmalloc/lib -lgc -lpthread

t-test1def: t-test1.c makefile
	    gcc -O2 -Wall -std=gnu99 -Wno-unused-result t-test1.c -o $@ -lpthread

t-test1tc: t-test1.c makefile
	    gcc -O2 -Wall -std=gnu99 -Wno-unused-result t-test1.c -o $@ -Wl,-rpath,/home/mkelly/tcmalloc/lib -L/home/mkelly/tcmalloc/lib -ltcmalloc_minimal -lpthread

t-test1je: t-test1.c makefile
	    gcc -O2 -Wall -std=gnu99 -Wno-unused-result t-test1.c -o $@ -Wl,-rpath,/home/mkelly/jemalloc/lib -L/home/mkelly/jemalloc/lib -ljemalloc -lpthread

t-test1dl: t-test1.c malloc.c makefile
	    gcc -O2 -Wall -std=gnu99 -Wno-unused-result t-test1.c malloc.c -o $@ -lpthread

t-test1pt3: t-test1.c makefile
	    gcc -O2 -Wall -std=gnu99 -Wno-unused-result t-test1.c -o $@ -L/home/mkelly/Downloads/ptmalloc3 -lptmalloc3 -lpthread

t-test1ll: t-test1.c makefile
	    gcc -O2 -Wall -std=gnu99 -Wno-unused-result t-test1.c -o $@ -L/home/mkelly/Downloads/lockless_allocator -lllalloc -lpthread

clean:
	    $(RM) -f mtestdef mtesttc mtestje mtestdl mtestpt3 mtestll mtesthoard mtestlockless ptestdef ptesttc ptestje mtestgc ptestgc t-test1def t-test1tc t-test1je t-test1dl t-test1pt3 t-test1ll

