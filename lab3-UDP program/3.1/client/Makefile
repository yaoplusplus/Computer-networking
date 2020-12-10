SHELL=cmd.exe
build: client.cpp
	g++ -g client.cpp -o client -lwsock32
	.\client.exe
clean:
	del client.exe
