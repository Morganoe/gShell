#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

pid_t Fork(void);
pid_t Wait(int *status);
pid_t Waitpid(pid_t pid, int *status, int options);
int Pipe(int pipefd[2]);
int Read(int fd, void *buf, size_t count);
int Write(int fd, const void *buf, size_t count);
int Open(const char *pathname, int flags);
int Close(int fd);
int Connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int Bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int Listen(int sockfd, int backlog);
int Accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
ssize_t Send(int sockfd, const void *buf, size_t len, int flags);
ssize_t Recv(int sockfd, void *buf, size_t len, int flags);
int Socket(int domain, int type, int protocol);
ssize_t Sendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen);
ssize_t Recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen);

