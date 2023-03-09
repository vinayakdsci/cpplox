CC = gcc

CFLAGS = -g -Wall

TARGET = cpplox

all: $(TARGET)

$(TARGET):
	$(CC) $(CFLAGS) -o $(TARGET).out src/*.c

clean:
	$(RM) -f .DS_Store
	$(RM) -rf *.dSYM/ 
veryclean:
	$(RM) -f *.out
	$(RM) -f .DS_Store
	$(RM) -rf *.dSYM/ 
	
