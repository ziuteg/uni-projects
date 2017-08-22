CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -O2

f: f.o
	$(CXX) $(CXXFLAGS) -o f f.o
f.o: f.cpp
	$(CXX) $(CXXFLAGS) -c f.cpp

clean:
	rm -f *.o

distclean:
	rm -f *.o e
