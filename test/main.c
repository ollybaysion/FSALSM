#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#define check_commit 449
#define is_committed 450

int main()
{
	int fd = open("./test.txt", O_RDWR);
	char buf[1024] = "Hello\0";

	syscall(check_commit, fd);
	write(fd, buf, 1024);

	fsync(fd);

	printf("is committed : %ld\n", syscall(is_committed, fd));
	return 0;
}
