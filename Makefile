CC = gcc
CFLAGS = -Wall -Wshadow -Wextra -O3 -g
LDFLAGS = 

all: is as disas $(patsubst %.s,%.o,$(shell ls *.s))
clean:
	$(RM) *.o is as disas
is: arch.o
as: arch.o
disas: arch.o

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@ -c
%: %.o
	$(CC) $(filter %.o,$^) -o $@ $(LDFLAGS)

%.o: %.s as
	./as $< $@

arch.o: arch.c arch.h
as.o: as.c arch.h
disas.o: disas.c arch.h
is.o: is.c arch.h
