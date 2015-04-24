CFLAGS = -g -I../phoenix/include 
LDFLAGS = -L../phoenix/lib -lphoenix -larmci 

CC = mpicc

all:
	$(CC) -o micro main.c access.c $(CFLAGS) $(LDFLAGS)

clean:
	rm -rf micro *.o

restartclean:
	rm -rf /mnt/ramdisk/mmap*
