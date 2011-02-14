# with apologies to how ugly of a makefile this is

CC = gcc
CFLAGS = -O2
OBJECTS = ps_main.o ps_sockets.o ps_list.o ps_data.o
BIN = powersecd

$(BIN) : $(OBJECTS)
	$(CC) $(CFLAGS) -o $(BIN) $(OBJECTS) 

powersec.o : ps_main.c ps_main.h
	$(CC) $(CFLAGS) -c  ps_main.c

ps_sockets.o : ps_sockets.c ps_sockets.h
	$(CC) $(CFLAGS) -c  ps_sockets.c

ps_list.o : ps_list.c ps_list.h
	$(CC) $(CFLAGS) -c ps_list.c

ps_data.o : ps_data.c ps_data.h
	$(CC) $(CFLAGS) -c ps_data.c

clean:
	-rm $(OBJECTS) 

#install: 
#	make

#uninstall:
#	-rm  $(OBJECTS) $(BIN)
#	
#
