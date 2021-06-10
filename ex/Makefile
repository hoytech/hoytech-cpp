W        = -Wall -Wextra
OPT      = -O2 -g
STD      = -std=c++17
INC      = -I..
CXXFLAGS = $(STD) $(OPT) $(W) $(INC) -fPIC $(XCXXFLAGS)
LDFLAGS  = -lpthread $(XLDFLAGS)

all: timer_test zerocopy_test hex_test

timer_test: timer_test.cpp ../hoytech/timer.h
	$(CXX) $(CXXFLAGS) timer_test.cpp $(LDFLAGS) -o $@

zerocopy_test: zerocopy_test.cpp ../hoytech/assert_zerocopy.h
	$(CXX) $(CXXFLAGS) zerocopy_test.cpp $(LDFLAGS) -o $@

hex_test: hex_test.cpp ../hoytech/hex.h
	$(CXX) $(CXXFLAGS) hex_test.cpp $(LDFLAGS) -o $@
