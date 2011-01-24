# with apologies to how ugly of a makefile this is

CC = gcc
CFLAGS = -O2
OBJECTS = powersec.o ps_sockets.o
BIN = powersecd

$(BIN) : $(OBJECTS)
	$(CC) $(CFLAGS) -o $(BIN) $(OBJECTS) 

powersec.o : powersec.c powersec.h
	$(CC) $(CFLAGS) -c  powersec.c

ps_sockets.o : ps_sockets.c ps_sockets.h
	$(CC) $(CFLAGS) -c  ps_sockets.c

clean:
	-rm $(OBJECTS) 

#install: 
#	make

#uninstall:
#	-rm  $(OBJECTS) $(BIN)
#	
