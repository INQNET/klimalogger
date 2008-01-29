
LIBOBJ = rw3600.o linux3600.o win3600.o

VERSION = 0.20

PROGS = dump_tfa decode_tfa

CFLAGS = -Wall -O2 -DVERSION=$(VERSION)
CC_LDFLAGS = -lm
CC_WINFLAG = 
# For Windows - comment the two line above and un-comment the two lines below.
#CC_LDFLAGS = -lm -lwsock32
#CC_WINFLAG = -mwindows

####### Build rules

all: $(PROGS)

dump_tfa : dump_tfa.o $(LIBOBJ)
	$(CC) $(CFLAGS) -o $@ dump_tfa.o $(LIBOBJ) $(CC_LDFLAGS)

decode_tfa : decode_tfa.o $(LIBOBJ)
	$(CC) $(CFLAGS) -o $@ decode_tfa.o $(LIBOBJ) $(CC_LDFLAGS)

clean:
	rm -f *~ *.o $(PROGS)


