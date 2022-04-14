#!make -f

CXX=clang++-9
CXXFLAGS=-pthread -std=c++2a
OBJECTS=server.cpp stack.c 

VALGRIND_FLAGS=-v --leak-check=full --show-leak-kinds=all  --error-exitcode=99

all: server
	./$^

client: clientC
	./client localhost
	
clientC: client.cpp
	$(CXX) $^ -o client

server: $(OBJECTS)
	$(CXX) $(CXXFLAGS) -ltbb $^ -o server

stack: stack.c
	$(CC) stack.c stack.h -c stack.o

valgrind: server
	valgrind --tool=memcheck $(VALGRIND_FLAGS) ./server | { egrep "lost| at " || true; }

clean:
	rm -f *.o main server ser cli a.out client original