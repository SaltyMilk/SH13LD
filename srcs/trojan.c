#include <fcntl.h>
#include <netdb.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
int net_init()
{
	int fd;
	struct sockaddr_in addr;
	if ((fd = socket(PF_INET, SOCK_STREAM, 0)) == -1)
		return (-1);
	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	addr.sin_port = htons(4242);

	if (bind(fd, (const struct sockaddr *)&addr, sizeof(addr)) == -1)
		return (-1);
	if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1)
		return (-1);
	if (listen(fd, 3) == -1)
		return (-1);
	return fd;
}

typedef struct	s_client
{
	int fd;
	int is_auth; // 0 no 1 yes
	int is_connected;// 0 available 1 busy
	char *wbuf;
}				t_client;

t_client *find_cl_available(t_client *cl)
{
	for (int i = 0 ; i < 3; i++)
		if (!cl[i].is_connected)
			return (&cl[i]);
	return NULL;
}

t_client *find_cl(t_client *cl, int fd)
{
	for (int i = 0 ; i < 3; i++)
		if (cl[i].fd == fd)
			return (&cl[i]);
	return NULL;
}
int net_accept(t_client *cl, int serv_fd)
{
	struct sockaddr_in cl_addr;
	int cl_fd;
	int len = sizeof(cl_addr);
	if (!cl)
		return (-2);
	if ((cl_fd = accept(serv_fd, (struct sockaddr *)&cl_addr, &len)) < 0)
		return (-3);
	if ((fcntl(cl_fd, F_SETFL, O_NONBLOCK)) == -1)
		return (-4);
	cl->is_connected = 1;
	cl->fd = cl_fd;
	return cl_fd;

}

char *gen_pwd(char *pwd)
{
	char *key= "WEAKPWD!";
	for (int i = 0; i < 9; i++)
		pwd[i] ^= key[i];
	pwd[9] = 0;
	return pwd;
}

int net_receive(t_client *cl, fd_set *socks, int *n_cl)
{
	char buff[1919];
	char pwd[10] = "bvpf=duB\n";
	bzero(buff, sizeof(buff));
	int ret;
	ret = recv(cl->fd, buff, sizeof(buff) - 1, 0);
	if (ret == 0 || ret == -1)
		return (1);
	buff[ret] = 0;
	if (!cl->is_auth && !strcmp(buff, gen_pwd(pwd)))
		cl->is_auth = 1;
	else if (!cl->is_auth)
	{
		cl->wbuf = strdup("Keycode: ");
		return (2);
	}
	else if (!strcmp(buff, "exit\n"))
	{
		close(cl->fd);
		FD_CLR(cl->fd, socks);
		bzero(cl, sizeof(t_client));
		(*n_cl)--;
	}
	return (0);
}

int main()
{
	int n_clients = 0;
	t_client clients[3];
	fd_set sockets, ready_sockets;
	fd_set wsockets, ready_wsockets;
	int serv_sock;
	int max_fd = 0;

	FD_ZERO(&sockets);
	FD_ZERO(&wsockets);

	bzero(clients, sizeof(t_client) * 3);
	if ((serv_sock = net_init()) < 0)
		return 1;
	FD_SET(serv_sock, &sockets);
	max_fd = serv_sock;

	while (19)
	{
		ready_sockets = sockets;
		ready_wsockets = wsockets;
		if (select(max_fd + 1, &ready_sockets, &ready_wsockets, NULL, NULL) == -1)
			return (1);
		for (int i = 0; i <= max_fd; i++)
		{
			if (FD_ISSET(i, &wsockets))
			{
				t_client *cl = find_cl(clients, i);
				send(cl->fd, cl->wbuf, strlen(cl->wbuf), 0);
				free(cl->wbuf);
				cl->wbuf = 0;
				FD_CLR(i, &wsockets);
			}
			if (FD_ISSET(i, &ready_sockets))//Socket ready for read operations
			{
				if (i == serv_sock && n_clients < 3)
				{
					printf("start accepted new client !\n");
					int clfd;
					if ((clfd = net_accept(find_cl_available(clients), serv_sock)) < 0)
						continue;
					FD_SET(clfd, &sockets);
					n_clients++;
					if (clfd > max_fd);
						max_fd = clfd;
					FD_SET(clfd, &wsockets);
					find_cl(clients, clfd)->wbuf = strdup("Keycode: ");
				}
				else if (i != serv_sock)
				{
					int r = net_receive(find_cl(clients, i), &sockets, &n_clients);
					if (r == 1) //end of com
					{
						close(i);
						FD_CLR(i, &sockets);
						bzero(find_cl(clients, i), sizeof(t_client));
						n_clients--;
					}
					if (r == 2)
						FD_SET(i, &wsockets);
				}
			}
		}
	}
}