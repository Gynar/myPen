.SUFFIXES: .c .o

CC= gcc
#INC= 
#CLAGS= -g $(INC)

SRCS= png_test.c
OBJS= png_test.o

TARGET= app

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS)

clean:
	rm -f $(TARGET) $(OBJS) *.bak

png_test.o: png_test.c
