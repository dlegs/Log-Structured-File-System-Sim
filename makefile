FLAGS = -Wall

all: initialize LFS

LFS: LFS.o imap.o segment.o inode.o pair.o
	gcc $(FLAGS) -g LFS.o imap.o segment.o inode.o pair.o -o LFS

initialize: initialize.o
	gcc $(FLAGS) -g initialize.o -o initialize

LFS.o: LFS.c imap.h segment.h inode.h defines.h
	gcc $(FLAGS) -g -c LFS.c -o LFS.o

initialize.o: initialize.c defines.h
	gcc $(FLAGS) -g -c initialize.c -o initialize.o

segment.o: segment.c segment.h pair.h defines.h
	gcc $(FLAGS) -g -c segment.c -o segment.o

imap.o: imap.c imap.h defines.h
	gcc $(FLAGS) -g -c imap.c -o imap.o

inode.o: inode.c inode.h defines.h
	gcc $(FLAGS) -g -c inode.c -o inode.o

pair.o: pair.c pair.h
	gcc $(FLAGS) -g -c pair.c -o pair.o

clean:
	rm -f *.o initialize LFS

