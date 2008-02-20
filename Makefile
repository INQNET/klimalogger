
LIBOBJ = eeprom.o linux3600.o data.o
PROGS = dump_tfa decode_tfa
CFLAGS = -Wall -O2


# Build rules
all: $(PROGS)

dump_tfa: dump_tfa.o $(LIBOBJ)

decode_tfa: decode_tfa.o $(LIBOBJ)

clean:
	rm -f *~ *.o $(PROGS)

.PHONY: clean

