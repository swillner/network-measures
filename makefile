CPP_FILES := betweenness_centrality.cpp flow_centrality.cpp gap.cpp
BINARIES := $(patsubst %.cpp,bin/%,$(CPP_FILES))
LD_FLAGS := $(shell mysql_config --libs)
CC_FLAGS := -std=c++0x $(shell mysql_config --cflags) -fopenmp -O -w

main: CXX = g++
main: $(BINARIES)

clean:
	rm -f $(BINARIES)

../binaries/%: network.cpp %.cpp
	mkdir -p bin
	$(CXX) $(CC_FLAGS) $(LD_FLAGS) -o $@ $^
