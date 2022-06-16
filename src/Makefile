all: server.exe c1.exe c2.exe c3.exe c4.exe


server.exe: server.c
	gcc server.c -pthread -o server



c1.exe: client.c client_monitor.c
	gcc client.c -Dc1  -o 1
	gcc client_monitor.c -Dc1 -o m1
c2.exe: client.c client_monitor.c
	gcc client.c -Dc2  -o 2
	gcc client_monitor.c -Dc2 -o m2
c3.exe: client.c client_monitor.c
	gcc client.c -Dc3  -o 3
	gcc client_monitor.c -Dc3 -o m3
c4.exe: client.c client_monitor.c
	gcc client.c -Dc4  -o 4
	gcc client_monitor.c -Dc4 -o m4


clean:
	rm 1
	rm 2
	rm 3
	rm 4
	rm server
	rm c1
	rm c2
	rm c3
	rm c4

