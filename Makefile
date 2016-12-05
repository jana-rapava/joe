PROGS = smoketest ondra sc zactor-example miraserver \
    fmq fmqcli fmqsrv \
    echoclient echobara echosrv \
    vit-spinka-client1 vit-spinka-file-client vit-spinka-file-server vit-spinka-file-zactor

CC = gcc
CXX= g++
LD = ld
prefix ?= /usr/local
CFLAGS = -I$(prefix)/include -std=c99 -D__EXTENSIONS__
CXXFLAGS = -I$(prefix)/include -std=c++99 -D__EXTENSIONS__
LIBS = -lczmq -lzmq -lmlm
LDFLAGS = -lc --entry main -L$(prefix)/lib
LDFLAGS_R ?= -R$(prefix)/lib
# Travis gcc does not like -R; hope real ld likes it
LDFLAGS += $(LDFLAGS_R)

all: $(PROGS)

$(foreach PROG,$(PROGS),\
    $(eval ${PROG}: ${PROG}.o))

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

$(foreach PROG,$(PROGS),\
    $(eval $(call memcheck_prog,${PROG})))

memcheck: all $(MEMCHECK_PROGS)

.PHONY: clean
clean:
	rm -f *.o $(PROGS) vgcore*
