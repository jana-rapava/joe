PROGS = smoketest ondra sc zactor-example miraserver \
    zactor-fmq zactor_file \
    fmq fmqcli fmqsrv \
    echoclient echobara echosrv \
    vit-spinka-client1 vit-spinka-file-client vit-spinka-file-server vit-spinka-file-zactor

JOE_OBJS = src/joe_proto.o
JOE_PROGS = src/vit-spinka-joe-zactor src/fmq-joe

CC = gcc
CXX= g++
#LD = ld -lc --entry main
LD = gcc
prefix ?= /usr/local
CFLAGS_DEBUG = -g3 -gdwarf-2 -O0
CFLAGS = -I$(prefix)/include -I./src -I./include -I./ -std=c99 -D__EXTENSIONS__ -D_GNU_SOURCE
CXXFLAGS = -I$(prefix)/include -I./src -I./include -I./ -std=c++99 -D__EXTENSIONS__ -D_GNU_SOURCE
LIBS = -lczmq -lzmq -lmlm
LDFLAGS = -L$(prefix)/lib
LDFLAGS_R ?= -R$(prefix)/lib
# Travis gcc does not like -R; hope real ld likes it
LDFLAGS += $(LDFLAGS_R)

ifdef DEBUG
CFLAGS += $(CFLAGS_DEBUG)
CXXFLAGS += $(CFLAGS_DEBUG)
endif

all: $(PROGS) $(JOE_PROGS)

$(foreach PROG,$(PROGS),\
    $(eval ${PROG}: ${PROG}.o))

$(foreach PROG,$(JOE_PROGS),\
    $(eval ${PROG}: ${PROG}.o $$(JOE_OBJS)))

%: %.o
	$(LD) $(LDFLAGS) $(LIBS) -o $@ $^

.c.o:
	$(CC) $(CFLAGS) -c -o $@ $<

.cc.o:
	$(CXX) $(CXXFLAGS) -c -o $@ $<

MEMCHECK_PROGS =
define memcheck_prog =
$(eval MEMCHECK_PROGS += memcheck-$(1))
memcheck-$(1): $(1)
	$$(LIBTOOL) --mode=execute valgrind --tool=memcheck \
		--leak-check=full --show-reachable=yes --error-exitcode=1 \
		./$$<
endef

$(foreach PROG,$(PROGS) $(JOE_PROGS),\
    $(eval $(call memcheck_prog,${PROG})))

memcheck: all $(MEMCHECK_PROGS)

.PHONY: clean
clean:
	rm -f *.o $(PROGS) $(JOE_PROGS) vgcore*
