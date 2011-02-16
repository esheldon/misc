CC = gcc
CCFLAGS = -I$(IDL_DIR)/external/include -fPIC 

default: test_autoglue.so

test_autoglue.so: test_autoglue.c
	$(CC) test_autoglue.c $(CCFLAGS) -shared -o test_autoglue.so
