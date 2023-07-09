oribli: oribli.cpp
	g++ -o oribli -std=c++17 oribli.cpp

install: oribli oribli.cmake
	cp oribli /usr/local/bin/
	mkdir -p /usr/lib/cmake/oribli
	cp oribli.cmake /usr/lib/cmake/oribli/oribli.cmake
