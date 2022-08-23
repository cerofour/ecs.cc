CXX=c++
CXXFLAGS=-g -std=c++17 -Wall -Wextra -pedantic
CPPFLAGS=-Iutils

SRC := ecs.cc \
	   utils/bits.cc

HDR := utils/metaprog.hpp \
	   utils/result.hpp \
	   utils/types.hpp \
	   utils/bits.hpp

OBJ := $(SRC:.cc=.o)

ecs: $(OBJ) $(HDR)
	$(CXX) $(CFLAGS) $(CXXFLAGS) $(CPPFLAGS) $^ -o $@

clean:
	rm $(OBJ) *.gch ecs
