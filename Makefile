# with apologies to how ugly of a makefile this is

CC = gcc
CFLAGS = -O2
OBJECTS = powersec.o
BIN = powersecd

$(BIN) : $(OBJECTS)
	$(CC) $(CFLAGS) -o $(BIN) $(OBJECTS) 

powersec.o : powersec.c powersec.h
	$(CC) $(CFLAGS) -c  powersec.c

clean:
	-rm $(OBJECTS) 

#install: 
#	make

#uninstall:
#	-rm  $(OBJECTS) $(BIN)
#	
