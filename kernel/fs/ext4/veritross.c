#include <linux/syscalls.h>
#include <linux/kernel.h>
#include <linux/fs.h>

#include "ext4_jbd2.h"

static int do_check_commit(unsigned int fd)
{
	struct fd f = fdget(fd);

	if(f.file) {
		struct inode *inode = f.file->f_mapping->host;
		EXT4_I(inode)->i_sstable = 1;
		EXT4_I(inode)->i_dirty = 1;
		ext4_dirty_inode(inode, 0);
	}

	return 0;
}

SYSCALL_DEFINE1(check_commit, unsigned int, fd)
{
	return do_check_commit(fd);
}

static int do_is_committed(unsigned int fd)
{
	struct fd f = fdget(fd);
	struct inode *inode = f.file->f_mapping->host;

	return EXT4_I(inode)->i_dirty;
}

SYSCALL_DEFINE1(is_committed, unsigned int, fd)
{
	return do_is_committed(fd);
}
