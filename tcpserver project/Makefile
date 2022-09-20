CC = gcc

  # compiler flags:
  #  -g    adds debugging information to the executable file
  #  -Wall turns on most, but not all, compiler warnings
CFLAGS  = -g -Wall

all:
	$(CC) $(CFLAGS) -o tcpclient tcpclient.c 
	$(CC) $(CFLAGS) -o async-tcpserver async-tcpserver.c 

starter:	
	$(CC) $(CFLAGS) client.c -o client
	$(CC) $(CFLAGS) server.c -o server

$(TARGET): $(TARGET).c
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).c

clean:
	$(RM) tcpclient
	$(RM) async-tcpserver