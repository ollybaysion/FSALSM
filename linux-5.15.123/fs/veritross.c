#include <linux/syscalls.h>
#include <linux/kernel.h>

static int do_check_commit(unsigned int fd)
{
	printk("[check_commit] fd : %d\n", fd);

	return fd;
}

SYSCALL_DEFINE1(check_commit, unsigned int, fd)
{
	return do_check_commit(fd);
}

