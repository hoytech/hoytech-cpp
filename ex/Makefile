W        = -Wall -Wextra
OPT      = -O2 -g
STD      = -std=c++17
INC      = -I../hoytech
CXXFLAGS = $(STD) $(OPT) $(W) $(INC) -fPIC $(XCXXFLAGS)
LDFLAGS  = -lpthread $(XLDFLAGS)

all: timer_test zerocopy_test

timer_test: timer_test.cpp ../timer.cpp ../hoytech/timer.h
	$(CXX) $(CXXFLAGS) ../timer.cpp timer_test.cpp $(LDFLAGS) -o $@

zerocopy_test: zerocopy_test.cpp ../hoytech/assert_zerocopy.h
	$(CXX) $(CXXFLAGS) zerocopy_test.cpp $(LDFLAGS) -o $@
