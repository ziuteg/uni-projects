CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra

transport: transport.o client.o
	$(CXX) $(CXXFLAGS) -o transport transport.o client.o
transport.o: transport.cpp client.h
	$(CXX) $(CXXFLAGS) -c transport.cpp -o transport.o
client.o: client.cpp client.h
	$(CXX) $(CXXFLAGS) -c client.cpp -o client.o

clean:
	rm -f *.o

distclean:
	rm -f *.o transport
