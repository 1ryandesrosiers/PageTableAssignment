CXX = g++
CC = gcc
CXXFLAGS = -g
CFLAGS = -g

pagingwithatc: main.o pageTable.o Level.o tlb.o tracereader.o log.o
	$(CXX) $(CXXFLAGS) -o pagingwithatc main.o pageTable.o Level.o tlb.o tracereader.o log.o

main.o: main.cpp pageTable.h Level.h tlb.h tracereader.h log.h
	$(CXX) $(CXXFLAGS) -c main.cpp

pageTable.o: pageTable.cpp pageTable.h Level.h
	$(CXX) $(CXXFLAGS) -c pageTable.cpp

Level.o: Level.cpp Level.h
	$(CXX) $(CXXFLAGS) -c Level.cpp

tlb.o: tlb.cpp tlb.h
	$(CXX) $(CXXFLAGS) -c tlb.cpp

tracereader.o: tracereader.c tracereader.h
	$(CC) $(CFLAGS) -c tracereader.c

log.o: log.c log.h
	$(CC) $(CFLAGS) -c log.c

clean:
	rm -f *.o pagingwithatc
run:
	./pagingwithatc -n 1000 -c 12 trace.tr 8 12
