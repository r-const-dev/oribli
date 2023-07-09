oribli: oribli.cpp
	g++ -o oribli -std=c++17 oribli.cpp

install:
	cp oribli /usr/local/bin/
