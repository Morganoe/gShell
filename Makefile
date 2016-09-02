CC = gcc -g
CFLAGS = -Wall

EXECS = gshell #ishell

all: $(EXECS)

#ishell: ishell.c wrappers.c wrappers.h dlist.c dlist.h dnode.c dnode.h
#	$(CC) $(CFLAGS) ishell.c dlist.c dnode.c wrappers.c -o ishell

gshell: gshell.c wrappers.c wrappers.h dlist.c dlist.h dnode.c dnode.h
	$(CC) $(CFLAGS) gshell.c dlist.c dnode.c wrappers.c -o gshell

clean:
	rm -f $(EXECS)
