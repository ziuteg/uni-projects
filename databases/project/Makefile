CXX = g++
CXXFLAGS = -std=c++14 -Wall -Wextra -lpqxx -lpq
VPATH = libs

conference: main.o command_handler.o db_functions.o
	$(CXX) $(CXXFLAGS) -o conference main.o command_handler.o db_functions.o
main.o: main.cpp command_handler.h json.hpp
	$(CXX) $(CXXFLAGS) -c main.cpp
command_handler.o: command_handler.cpp command_handler.h db_functions.h json.hpp
	$(CXX) $(CXXFLAGS) -c command_handler.cpp
db_functions.o: db_functions.cpp db_functions.h json.hpp
	$(CXX) $(CXXFLAGS) -c db_functions.cpp

clean:
	rm -f *.o

distclean:
	rm -f *.o conference
