CXX = g++
CXXFLAGS = -std=c++17 -Wall

# Build the server
all:
	$(CXX) $(CXXFLAGS) main.cpp -o server

# Remove compiled binary
clean:
	rm -f server
