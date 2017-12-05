LIBRARIES = -lcoap-1
INCLUDE = -I /usr/local/include/coap
CXXFLAGS = -DWITH_POSIX
SRC = server.c
TARGET = server
CC = gcc

all: server
		
server:
	$(CC) $(SRC) $(INCLUDE) $(LIBRARIES) $(CXXFLAGS) -o $(TARGET)
			
