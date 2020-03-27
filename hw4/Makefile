# all: main.cpp run

# SOURCES    = main.cpp
# OBJECTS    = $(SOURCES:.cpp=.o)

# .cpp.o:
# 	g++ -w -O3 -c $< -o $@

# run: main.o
# 	g++ $(OBJECTS) $(LDFLAGS) -o $@

# clean:
# 	rm -f *.o


########

# all: main.cpp run

# SOURCES    = main.cpp animation.cpp utilities.cpp
# OBJECTS    = $(SOURCES:.cpp=.o)

# .cpp.o:
# 	g++ -w -c -g $< -o $@

# run: main.o animation.o utilities.o
# 	g++ $(OBJECTS) $(LDFLAGS) -o $@

# clean:
# 	rm -f *.o run


########

all: main.cpp run

SOURCES    = main.cpp animation.cpp utilities.cpp
OBJECTS    = $(SOURCES:.cpp=.o)

.cpp.o:
	g++ -w -c -O3 $< -o $@

run: main.o animation.o utilities.o
	g++ $(OBJECTS) $(LDFLAGS) -o $@

clean:
	rm -f *.o run


