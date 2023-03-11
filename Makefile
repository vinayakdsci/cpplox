CC = gcc

CFLAGS = -g -Wall

TARGET = cpplox

all: $(TARGET)

$(TARGET):
	$(CC) $(CFLAGS) -o $(TARGET).out src/*.c

debug:
	@echo "BUILT WITH DEBUG FLAGS"
	$(CC) $(CFLAGS) -DDEBUG_TRACE_EXECUTION -DDEBUG_PRINT_CODE -o $(TARGET).out src/*.c
clean:
	$(RM) -f .DS_Store
	$(RM) -rf *.dSYM/ 
veryclean:
	$(RM) -f *.out
	$(RM) -f .DS_Store
	$(RM) -rf *.dSYM/ 
	
