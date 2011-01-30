# with apologies to how ugly of a makefile this is

CC = gcc
CFLAGS = -O2
OBJECTS = powersec.o ps_sockets.o ps_list.o
BIN = powersecd

$(BIN) : $(OBJECTS)
	$(CC) $(CFLAGS) -o $(BIN) $(OBJECTS) 

powersec.o : powersec.c powersec.h
	$(CC) $(CFLAGS) -c  powersec.c

ps_sockets.o : ps_sockets.c ps_sockets.h
	$(CC) $(CFLAGS) -c  ps_sockets.c

ps_list.o : ps_list.c ps_list.h
	$(CC) $(CFLAGS) -c ps_list.c

clean:
	-rm $(OBJECTS) 

#install: 
#	make

#uninstall:
#	-rm  $(OBJECTS) $(BIN)
#	
