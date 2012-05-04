# define target filename
LIB_TARGET = libcapt.a
SAMPLE_TARGET = captGen

# compiler common define
CC 				= g++
AR 				= ar
CPPFLAGS 	= -I../../include -g -O2

#All object files
OBJS =\
	captFontFile.o \
	captGenerator.o \
	captRLE.o
	
all: $(LIB_TARGET) $(SAMPLE_TARGET)


captGen.o : ../../samples/captGen.cpp
	$(CC) $(CPPFLAGS) -c $< -o $@

%.o : ../../src/%.cpp
	$(CC) $(CPPFLAGS) -c $< -o $@

$(LIB_TARGET) : $(OBJS)
	$(AR) sru $(LIB_TARGET) $^
	ranlib $(LIB_TARGET)

$(SAMPLE_TARGET) : captGen.o $(LIB_TARGET)
	$(CC) captGen.o $(LIB_TARGET) -o $@

clean:
	rm -f $(LIB_TARGET)
	rm -f $(SAMPLE_TARGET)
	rm -f *.o
	rm -f *.a

