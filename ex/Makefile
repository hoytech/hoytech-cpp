W        = -Wall -Wextra
OPT      = -O2 -g
STD      = -std=c++11
INC      = -I../hoytech
CXXFLAGS = $(STD) $(OPT) $(W) $(INC) -fPIC $(XCXXFLAGS)
LDFLAGS  = -lpthread $(XLDFLAGS)

all: timer_test

timer_test: timer_test.cpp
	$(CXX) $(CXXFLAGS) ../timer.cpp timer_test.cpp $(LDFLAGS) -o $@
