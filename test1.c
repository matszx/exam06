#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>

#define	MAX_MSG_SIZE	1000000
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
	if (!msg)
		msg = "Fatal error\n";
	write(2, msg, strlen(msg));
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

	if (bind(sockfd, (const struct sockaddr*)&servaddr, sizeof(servaddr)) != 0)
		err(NULL);
	
	listen(sockfd, 10);
	maxfd = sockfd;
	while (1)
	{
		read_set = write_set = current;
		if (select(maxfd + 1, &read_set, &write_set, NULL, NULL) == -1)
			err(NULL);
		for (int fd = 0; fd <= maxfd; fd++)
		{
			if (FD_ISSET(fd, &read_set))
			{
				if (fd == sockfd)
				{
					struct sockaddr_in	cli = {0};
					socklen_t			len = sizeof(cli);

					connfd = accept(sockfd, (struct sockaddr*)&cli, &len);
					if (connfd < 0)
						err(NULL);
					if (connfd > maxfd)
						maxfd = connfd;
					clients[connfd].id = current_id++;
					FD_SET(connfd, &current);
					sprintf(send_buffer, "server: client %d just arrived\n", clients[connfd].id);
					send_broadcast(connfd);
				}
				else
				{
					int	ret = recv(fd, recv_buffer, MAX_MSG_SIZE, 0);
					if (ret <= 0)
					{
						sprintf(send_buffer, "server: client %d just left\n", clients[fd].id);
						send_broadcast(fd);
						FD_CLR(fd, &current);
						close(fd);
					}
					else
					{
						for (int i = 0, j = strlen(clients[fd].msg); i < ret; i++, j++)
						{
							clients[fd].msg[j] = recv_buffer[i];
							if (clients[fd].msg[j] == '\n')
							{
								clients[fd].msg[j] = 0;
								sprintf(send_buffer, "client %d: %s\n", clients[fd].id, clients[fd].msg);
								send_broadcast(fd);
								bzero(clients[fd].msg, strlen(clients[fd].msg));
								j = -1;
							}
						}
					}
				}
			}
		}
	}
}