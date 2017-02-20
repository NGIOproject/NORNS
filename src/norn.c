#include<stdio.h>
#include<sys/un.h>
#include<sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdint.h>

#define SOCKET_NAME "dloom_socket" 

struct task{
	pid_t pid;
	uint64_t taskId;
	const char *filePath;
};

void initialize(){
	int sock;
	struct sockaddr_un server;
	char buff[1024] = "hola";
	struct task t;
	t.pid = 3;
	t.taskId = 4;

	sock = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sock < 0) {
		perror("opening stream socket");
		exit(1);
	}

	server.sun_family = AF_UNIX;
	strcpy(server.sun_path, SOCKET_NAME);

	if (connect(sock, (struct sockaddr *) &server, sizeof(struct sockaddr_un)) < 0) {
        if (close(sock) < 0)
        	exit(1);
        perror("connecting stream socket");
        exit(1);
    }
    if (write(sock, &t, sizeof(t)) < 0)
        perror("writing on stream socket");
    close(sock);
}
