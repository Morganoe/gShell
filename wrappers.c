#include "wrappers.h"
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <stdio.h>

pid_t Fork(void)
{
	pid_t pid;
	if((pid = fork()) < 0)
	{
		perror("Fork failed!");
		exit(-1);
	}
	return pid;
}

pid_t Wait(int *status)
{
	pid_t pid;
	if((pid = wait(status)) == -1)
	{
		perror("Wait failed!");
		exit(-1);
	}
	return pid;
}

pid_t Waitpid(pid_t pid, int *status, int options)
{
	pid_t npid;
	if((npid = waitpid(pid, status, options)) == -1)
	{
		perror("Waitpid failed!");
		exit(-1);
	}
	return npid;
}

int Pipe(int pipefd[2])
{
	if(pipe(pipefd) == -1)
	{
		perror("Pipe not created!");
		exit(-1);
	}
	return 1;
}

int Read(int fd, void *buf, size_t count)
{
	int c_read;
	if((c_read = read(fd, buf, count)) == -1)
	{
		perror("Read failed!");
		return -1;
	}
	return c_read;
}

int Write(int fd, const void *buf, size_t count)
{
	int c_write;
	if((c_write = write(fd, buf, count)) == -1)
	{
		perror("Write failed!");
		return -1;
	}
	return c_write;
}

int Open(const char *pathname, int flags)
{
	int fd;
	if((fd = open(pathname, flags)) == -1)
	{
		perror("Open failed!");
		return -1;
	}
	return fd;
}

int Close(int fd)
{
	if(close(fd) == -1)
	{
		perror("Close failed!");
		exit(-1);
	}
	return 0;
}

int Connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
	if(connect(sockfd, addr, addrlen) == -1)
	{
		perror("Connect failed!");
		exit(-1);
	}
	return 0;
}

int Bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
	if(bind(sockfd, addr, addrlen) == -1)
	{
		perror("Bind failed!");
		exit(-1);
	}
	return 0;
}

int Listen(int sockfd, int backlog)
{
	if(listen(sockfd, backlog) == -1)
	{
		perror("Listen failed!");
		exit(-1);
	}
	return 0;
}

int Accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
	int acc;
	if((acc = accept(sockfd, addr, addrlen)) == -1)
	{
		perror("Accept failed!");
		exit(-1);
	}
	return acc;
}

ssize_t Send(int sockfd, const void *buf, size_t len, int flags)
{
	int num_c;
	if((num_c = send(sockfd, buf, len, flags)) == -1)
	{
		perror("Send failed!");
		exit(-1);
	}
	return num_c;
}

ssize_t Recv(int sockfd, void *buf, size_t len, int flags)
{
	int bytes_r;
	if((bytes_r = recv(sockfd, buf, len, flags)) == -1)
	{
		perror("Receive failed!");
		exit(-1);
	}
	return bytes_r;
}

int Socket(int domain, int type, int protocol)
{
	int fd;
	if((fd = socket(domain, type, protocol)) == -1)
	{
		perror("Socket failed!");
		exit(-1);
	}
	return fd;
}

ssize_t Sendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen)
{
	int chars;
	if((chars = sendto(sockfd, buf, len, flags, dest_addr, addrlen)) == -1)
	{
		perror("Sendto failed!");
		exit(-1);
	}
	return chars;	
}

ssize_t Recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen)
{
	int chars;
	if((chars = recvfrom(sockfd, buf, len, flags, src_addr, addrlen)) == -1)
	{
		perror("Recvfrom failed!");
		exit(-1);
	}
	return chars;	
}
