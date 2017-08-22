CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra

webserver: main.o server.o utils.o
	$(CXX) $(CXXFLAGS) -o webserver main.o server.o utils.o

main.o: main.cpp server.h
	$(CXX) $(CXXFLAGS) -c main.cpp -o main.o

server.o: server.cpp server.h utils.h
	$(CXX) $(CXXFLAGS) -c server.cpp -o server.o

utils.o: utils.cpp utils.h
	$(CXX) $(CXXFLAGS) -c utils.cpp -o utils.o

clean:
	rm -f *.o

distclean:
	rm -f *.o webserver
