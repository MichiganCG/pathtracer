SOURCES = $(wildcard *.cpp)
OBJECTS = $(SOURCES:%.cpp=%.o)
OUT     = pathtracer
CXX     = g++
FLAGS   = -std=c++20 -Wall

release: FLAGS += -O3 -DNDEBUG
release: $(OUT)

debug: FLAGS += -g3 -DDEBUG
debug: $(OBJECTS)
	$(CXX) $(FLAGS) $(OBJECTS) -o $(OUT)_debug

$(OUT): $(OBJECTS)
	$(CXX) $(FLAGS) $(OBJECTS) -o $(OUT)

%.o: %.cpp
	$(CXX) $(FLAGS) -c $*.cpp

all: debug release

clean:
	rm -f $(OBJECTS) $(OUT) $(OUT)_debug
