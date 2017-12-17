.SUFFIXES: .c .o

CC= gcc
#INC= 
#CLAGS= -g $(INC)

NM0= png_main

SRCS= $(NM0).c
OBJS= $(NM0).o

TARGET= app

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS)

clean:
	rm -f $(TARGET) $(OBJS) *.bak

$(NM0).o: $(NM0).c
