#!make -f

CXX=clang++-9
CXXFLAGS=-pthread -std=c++2a
OBJECTS=server.cpp stack.c 

VALGRIND_FLAGS=--leak-check=full

test: testC stack
	$(CC) stack.o test.o -o test

testC: 
	$(CC) test.c -c

all: server
	./$^

client: clientC
	./client localhost
	
clientC: client.cpp
	$(CXX) $^ -o client

server: $(OBJECTS)
	$(CXX) $(CXXFLAGS) -ltbb $^ -o server

stack: stack.c
	$(CC) stack.c stack.h -c 

valgrind: server
	valgrind $(VALGRIND_FLAGS) ./server

clean:
	rm -f *.o main server ser cli a.out client original stack.h.gch tests test