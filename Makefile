CLIENT=subscriber
SERVER=server

UTILS_PATH = utils/lib/
SOURCES_SERVER = server.c \
	$(UTILS_PATH)queue.c $(UTILS_PATH)list.c $(UTILS_PATH)buffer.c $(UTILS_PATH)action.c
SOURCES_CLIENT = subscriber.c \
	$(UTILS_PATH)queue.c $(UTILS_PATH)list.c $(UTILS_PATH)client.c

LIBRARY=nope
LIBPATHS=.
INCPATHS=
LDFLAGS=
CFLAGS=-c -Wall
CC=gcc

#Automatic generation of some important lists
OBJECTS_SERVER=$(SOURCES_SERVER:.c=.o)
OBJECTS_CLIENT=$(SOURCES_CLIENT:.c=.o)
INCFLAGS=$(foreach TMP,$(INCPATHS),-I$(TMP))
LIBFLAGS=$(foreach TMP,$(LIBPATHS),-L$(TMP))

#Set up the output file names for the different output types
BINARY_SERVER=$(SERVER)
BINARY_CLIENT=$(CLIENT)

.PHONY: all run_tests run_tests_permissions run_server run_subscriber cleanBIN cleanOBJ clean

all: $(SOURCES_SERVER) $(BINARY_SERVER) $(SOURCES_CLIENT) $(BINARY_CLIENT)

run_tests:
	python3 test.py

run_tests_permissions:
	sudo python3 test.py

run_server:
	./server ${PORT}

run_subscriber:
	./subscriber ${ID_CLIENT} ${IP_SERVER} ${PORT_SERVER}

$(BINARY_SERVER): $(OBJECTS_SERVER)
	$(CC) $(LIBFLAGS) $(OBJECTS_SERVER) $(LDFLAGS) -o $@

$(BINARY_CLIENT): $(OBJECTS_CLIENT)
	$(CC) $(LIBFLAGS) $(OBJECTS_CLIENT) $(LDFLAGS) -o $@

.c.o:
	$(CC) $(INCFLAGS) $(CFLAGS) -fPIC $< -o $@

cleanBIN:
	rm -f $(BINARY_SERVER) $(BINARY_CLIENT)

cleanOBJ:
	rm -f $(OBJECTS_SERVER) $(OBJECTS_CLIENT)
	
clean:
	rm -f $(BINARY_SERVER) $(BINARY_CLIENT) $(OBJECTS_SERVER) $(OBJECTS_CLIENT)