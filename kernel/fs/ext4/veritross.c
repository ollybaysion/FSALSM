#include <linux/syscalls.h>
#include <linux/kernel.h>
#include <linux/fs.h>

#include "ext4_jbd2.h"

SYSCALL_DEFINE1(check_commit, unsigned int, fd)
{
	return 15;
}

SYSCALL_DEFINE1(is_committed, unsigned int, fd)
{
	return 20;
}
