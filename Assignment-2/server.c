// Server side C/C++ program to demonstrate Socket programming
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <pwd.h>
#include <sys/wait.h>
#define PORT 8080

int main(int argc, char const *argv[])
{
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
    char *hello = "Hello from server";

    struct passwd* pwd;
    pid_t processId;
    
   if (strcmp(argv[0], "child") == 0)
    {
        int new_socket_copy = atoi(argv[1]);
        valread = read(new_socket_copy, buffer, 1024);
        printf("Read %d bytes: %s\n", valread, buffer);
        send(new_socket_copy, hello, strlen(hello), 0);
        printf("Hello message sent\n");
        exit(0);
    }

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
	&opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );

    if (bind(server_fd, (struct sockaddr *)&address,
	sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
                       (socklen_t*)&addrlen))<0)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    processId = fork();

    if(processId < 0)
    {
	    perror("Fork Failed\n");
	    exit(EXIT_FAILURE);
    }
    else if(processId == 0) {
    printf("Child Process \n");
    pwd = getpwnam("nobody");
    printf("pid of nobody is %d\n", pwd->pw_uid);
    int duplicate_soc = dup(new_socket);
    if(duplicate_soc == -1) {
            perror("Socket duplication failed\n");
	    exit(EXIT_FAILURE);
    }
    //Dropping privileges
    setuid(pwd -> pw_uid);
    printf("Privilege has been dropped successfully\n");
    
    // re-exec the child process after fork
    char intToStr[10];
    //snprintf(intToStr,10,"%d",duplicate_soc);
     snprintf(intToStr,10,"%d",new_socket);
    char *args[] = {"child",intToStr,NULL};
    int status = execvp(argv[0], args);
    if(status < 0) {
        perror("re-exec failed!\n");
        exit(EXIT_FAILURE);
      }
    }
    wait(NULL);
    return 0;
}