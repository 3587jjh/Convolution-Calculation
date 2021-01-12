all: program1 program2 program3

program1: program1.cpp
			g++ -std=c++11 -o program1 program1.cpp

program2: program2.cpp
			g++ -std=c++11 -o program2 program2.cpp

program3: program3.cpp
			g++ -std=c++11 -o program3 program3.cpp -lpthread

clean:
		rm program1 program2 program3
