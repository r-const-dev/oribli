oribli: oribli.cpp
	g++ -o oribli -std=c++17 oribli.cpp

install: oribli
	cp oribli /usr/local/bin/
