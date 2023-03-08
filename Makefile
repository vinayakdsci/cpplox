CC = gcc-12

CFLAGS = -g -Wall

TARGET = cpplox

all: $(TARGET)

$(TARGET):
	$(CC) $(CFLAGS) -DDEBUG_TRACE_EXECUTION -o $(TARGET).out src/*.c

clean:
	$(RM) -f .DS_Store
	$(RM) -rf *.dSYM/ 
veryclean:
	$(RM) -f *.out
	$(RM) -f .DS_Store
	$(RM) -rf *.dSYM/ 
	
