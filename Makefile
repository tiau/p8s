cc ?= ${CC}
cc ?= gcc
WEXTRA := -pedantic -Wsuggest-attribute=pure -Wsuggest-attribute=const -Wsuggest-attribute=noreturn -Wformat-nonliteral -Wformat-security -Winit-self -Wparentheses -Wuninitialized -Wstrict-overflow=5 -Wshadow -Wunsafe-loop-optimizations -Wbad-function-cast -Wcast-qual -Wcast-align -Wwrite-strings -Wjump-misses-init -Wlogical-op -Winline -Wvector-operation-performance -Wdisabled-optimization
WFLAGS := -Wall -Wextra -Wfloat-equal -Wpointer-arith -Wbad-function-cast -Wcast-align -Wwrite-strings -Wmissing-prototypes -Wmissing-declarations -Wmissing-noreturn -Wmissing-format-attribute -Wredundant-decls -Wnested-externs -Winvalid-pch -Wdisabled-optimization -Wstrict-prototypes -Wno-vla -Wno-unused-parameter -Wstrict-aliasing=2
CFLAGS := -march=native ${CFLAGS}
DFLAGS := -ggdb -DDEBUG -fstrict-aliasing
RFLAGS := -O3 -fomit-frame-pointer -ffast-math -DNDEBUG -flto -fwhole-program -fivopts -funsafe-loop-optimizations
SFLAGS := -std=gnu11 -D_GNU_SOURCE -lpthread -lrt -lm
PFLAGS := -fprofile-correction -fprofile-use -funroll-loops -funswitch-loops -fbranch-target-load-optimize 
TFLAGS := -pg -g -fprofile-arcs -ftest-coverage
PROFILE := -fno-omit-frame-pointer -fno-inline-functions -fno-inline-functions-called-once -fno-optimize-sibling-calls -fno-inline

FILES := engine.c io.c mhash.c movegen.c p8.c plist.c poker.c
AIS := ai/human.c ai/first.c ai/random.c ai/shedder.c ai/monte.c ai/judge.c ai/stacked.c ai/draw.c ai/cheat.c
FAI := ${FILES} ${AIS}

all: ${FAI}
	make debug

release: ${FAI}
	${cc} ${WFLAGS} ${CFLAGS} ${RFLAGS} ${FAI} ${SFLAGS} -s -o p8

debug: ${FAI}
	${cc} -DMONTE_VERBOSE ${WFLAGS} ${CFLAGS} ${DFLAGS} ${FAI} ${SFLAGS} -o p8

warn: ${FAI}
	${cc} -DMONTE_VERBOSE ${WFLAGS} ${WEXTRA} ${CFLAGS} ${RFLAGS} ${DFLAGS} ${FAI} ${SFLAGS} -o p8

loud: ${FAI}
	@CFLAGS="${CFLAGS} ${RFLAGS} -DMONTE_VERBOSE" make debug

louder: ${FAI}
	@CFLAGS="${CFLAGS} ${RFLAGS} -DJUDGE_VERBOSE" make debug

includegraph: ${FAI}
	echo -e "digraph {\n`grep -r include * | egrep -v README\|TODO\|Makefile\|ccglue\|cscope\|Binary\|\.c:# | sed -e 's/^\([^:]*\):[^\ ]*\ \(.*\)/"\1"->\2/g' -e 's/\.\.\///' -e 's/ai\///' -e 's/>\->/*>->/'`}" | xdot - &

profile: ${FAI}
	${cc} -fprofile-generate ${CFLAGS} ${WFLAGS} ${RFLAGS} ${FAI} ${SFLAGS} -o p8
	./p8 -m67 >/dev/null
	${cc} ${PFLAGS} ${CFLAGS} ${WFLAGS} ${RFLAGS} ${FAI} ${SFLAGS} -s -o p8
	@make clean

gprof: ${FAI}
	${cc} -fprofile-generate ${TFLAGS} ${CFLAGS} ${WFLAGS} ${RFLAGS} ${PROFILE} ${FAI} ${SFLAGS} -o p8
	./p8 -g400 -m5432 >/dev/null
	echo "gcov'ing...`for a in ${FAI} ; do gcov -b $$a > /dev/null ; done`"
	gprof -bcz p8 | gprof2dot -n.1 -e.02 | sed 's/digraph\ /digraph\ callgrind\ /' | xdot - &
	@sleep 1
	@make clean

callgrind: ${FAI}
	${cc} -g -DMONTE_VERBOSE ${CFLAGS} ${WFLAGS} ${RFLAGS} ${PROFILE} ${FAI} ${SFLAGS} -o p8
	@valgrind --tool=callgrind --collect-systime=yes ./p8 -m0543 -vv
	cat callgrind.out.* | gprof2dot -n.1 -e.02 -f callgrind | sed 's/digraph\ /digraph\ callgrind\ /' | xdot - &
	@sleep 1
	@make clean

clean:
	@rm -f *.expand *.png *.gcda *.s *.gcov *.gcno gmon.out *.tmp callgrind.out.*
