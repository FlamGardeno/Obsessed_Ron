test1.o: test1.cpp map.h globals.h
	g++ -I. -c test1.cpp

map.o: map.cpp map.h globals.h
	g++ -I. -c map.cpp

globals.o: globals.cpp globals.h
	g++ -I. -c globals.cpp

test1: test1.o map.o globals.o
	g++ test1.o map.o globals.o -o test1

T1: test1
	./test1

clean:
	rm -f *.o test1
