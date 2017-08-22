CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -O2

e: e.o
	$(CXX) $(CXXFLAGS) -o e e.o
e.o: e.cpp
	$(CXX) $(CXXFLAGS) -c e.cpp

clean:
	rm -f *.o

distclean:
	rm -f *.o e
