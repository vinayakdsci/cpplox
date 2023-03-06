CC = gcc-12

CFLAGS = -g -Wall -Wextra

TARGET = cpplox

all: $(TARGET)

$(TARGET):
	$(CC) $(CFLAGS) -o $(TARGET).out src/*.c

clean:
	$(RM) -f *.out .DS_Store
	$(RM) -rf *.dSYM/ 
