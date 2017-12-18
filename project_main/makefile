.SUFFIXES: .c .o
CC = gcc
CXXFLAGS = -DWITH_POSIX -pthread -lm

LIBS = -lcoap-1
INC = -I /usr/local/include/coap

SRCS = sketch.c coap_impl.c gpng_impl.c
OBJS = sketch.o coap_impl.o gpng_impl.o

TARGET = app

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS) $(INC) $(LIBS) $(CXXFLAGS)

clean:
	rm -f $(TARGET) $(OBJS)

#sketch.o: sketch.c gpng_impl.h coap_impl.h
#	$(CC) -c sketch.c
#gpng_tmpl.o: gpng_impl.c gpng_impl.h
#	$(CC) -c gpng_impl.c
#coap_impl.o: coap_impl.c coap_impl.h
#	$(CC) -c coap_impl.c 



