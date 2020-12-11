SHELL=cmd.exe
build: client.cpp
	g++ -g client.cpp -o client -lwsock32 -std=c++11
	.\client.exe
clean:
	del client.exe
test: test.cpp
	g++ -g test.cpp -o test
	.\test.exe
