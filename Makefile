CXX=c++
CXXFLAGS=-g -std=c++17 -Wall -Wextra -pedantic
CPPFLAGS=-Iutils

AR = ar
ARFLAGS = rcs

SRC := utils/bits.cc

HDR := utils/metaprog.hpp \
	   utils/result.hpp \
	   utils/types.hpp \
	   utils/bits.hpp

OBJ := $(SRC:.cc=.o)
CHDR := $(addsuffix .gch,$(HDR))

LIBNAME = libecs
MAJOR_VERSION = 0
MINOR_VERSION = 0
SUB_VERSION = 0

VERSION_SUFFIX = $(MAJOR_VERSION).$(MINOR_VERSION).$(SUB_VERSION)

static: $(OBJ) $(HDR)
	$(CXX) $(CFLAGS) $(CXXFLAGS) $(CPPFLAGS) $^ -c
	$(AR) $(ARFLAGS) $(LIBNAME).a $(OBJ) 

shared: $(OBJ) $(HDR)
	$(CXX) -c -fPIC $(CFLAGS) $(CXXFLAGS) $(CPPFLAGS) $^
	$(CXX) -shared -Wl,-soname,$(LIBNAME).so.$(MAJOR_VERSION) -o $(LIBNAME).so.$(VERSION_SUFFIX) $(OBJ)
	ln -s $(LIBNAME).so.$(MAJOR_VERSION) $(LIBNAME).so

clean:
	rm $(OBJ) \
		$(CHDR) \
		$(LIBNAME).so.$(VERSION_SUFFIX) \
		$(LIBNAME).so \
		$(LIBNAME).a
