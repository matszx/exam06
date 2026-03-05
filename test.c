#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>

#define	MAX_MSG_SIZE	100000
#define	MAX_CLIENTS		1024

typedef struct s_client
{
	int		id;
	char	msg[MAX_MSG_SIZE];
}	t_client;

t_client	clients[MAX_CLIENTS];
int			maxfd = 0, current_id = 0;

fd_set		read_set, write_set, current;
char		send_buffer[MAX_MSG_SIZE], recv_buffer[MAX_MSG_SIZE];

void	err(char* msg)
{
	if (msg)
		write(2, msg, strlen(msg));
	else
		write(2, "Fatal error\n", 13);
	exit(1);
}

void	send_broadcast(int accepted)
{
	for (int fd = 0; fd <= maxfd; fd++)
	{
		if (FD_ISSET(fd, &write_set) && fd != accepted)
			send(fd, send_buffer, strlen(send_buffer), 0);
	}
}

int		main(int ac, char** av)
{
	if (ac != 2)
		err("Wrong number of arguments\n");

	int	sockfd, connfd;

	sockfd = socket(2, SOCK_STREAM, 0);
	FD_ZERO(&current);
	FD_SET(sockfd, &current);

	struct sockaddr_in	servaddr = {0};

	servaddr.sin_family = 2;
	servaddr.sin_addr.s_addr = htonl(0x7f000001);
	servaddr.sin_port = htons(atoi(av[1]));


}