linker: main.cpp main.h errorevent.cpp errorevent.h
	g++ -std=c++11 -g main.cpp errorevent.cpp -o linker


clean: 
	rm -rf linker*
	rm -rf ./output/*
