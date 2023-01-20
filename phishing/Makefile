all: phisher.out

phisher.out: phisher.cpp convert.o
	g++ phisher.cpp convert.o -o phisher.out

convert.o: convert.cpp
	g++ -c convert.cpp -o convert.o