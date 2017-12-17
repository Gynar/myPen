CC = gcc
CXXFLAGS = -DWITH_POSIX

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

sketch.o: sketch.c
coap_impl.o: coap_impl.c
gpng_impl.o: gpng_impl.c



