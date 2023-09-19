#include <linux/syscalls.h>
#include <linux/kernel.h>
#include <linux/fs.h>

#include "ext4_jbd2.h"

static unsigned int do_check_commit(unsigned int fd)
{
	struct fd f = fdget(fd);
	struct inode *inode = f.file->f_mapping->host;
	struct ext4_inode_info *ei = EXT4_I(inode);

	printk("[do_check_commit]\n");
	ei->i_sstable = 1;
	ei->i_dirty = 1;

	ext4_dirty_inode(inode, 0);
	return 0;
}

SYSCALL_DEFINE1(check_commit, unsigned int, fd)
{
	return do_check_commit(fd);
}

static unsigned int do_is_committed(unsigned int fd)
{
	struct fd f = fdget(fd);
	struct inode *inode = f.file->f_mapping->host;
	struct ext4_inode_info *ei = EXT4_I(inode);

	return ei->i_dirty;
}

SYSCALL_DEFINE1(is_committed, unsigned int, fd)
{
	return do_is_committed(fd);
}
