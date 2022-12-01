prodcon: prodcon.cpp tands.cpp
	g++ -o prodcon prodcon.cpp tands.cpp -pthread
clean:
	rm prodcon