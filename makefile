all: main


main: main.cpp Estimator.h trace.h
	g++ -o main main.cpp -lm -std=c++17


clean:
	rm *.o main
