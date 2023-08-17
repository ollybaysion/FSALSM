#include <linux/syscalls.h>
#include <linux/kernel.h>
#include <linux/fs.h>

#include "ext4_jbd2.h"

/*
   - fs/sync.c
	sys_fsync
	do_fsync
	vfs_fsync
	vfs_fsync_range
	mark_inode_dirty_sync
	file->f_op->fsync

	- include/linux/fs.h
	mark_inode_dirty

	- fs/fs-writeback.c
	__mark_inode_dirty
	dirty_inode

	- fs/ext4/inode.c
	ext4_dirty_inode
	ext4_mark_inode_dirty
	__ext4_mark_inode_dirty
	ext4_mark_iloc_dirty

	- fs/ext4/fsync.c
	ext4_sync_file
	ext4_fsync_journal
	ext4_fc_commit
	ext4_fc_perform_commit
	ext4_fc_submit_inode_data_all
	ext4_fc_wait_inode_data_all

	- fs/jbd2/journal.c
	*kjournald2
	*jbd2_journal_commit_transaction

	- fs/jbd2/commit.c
	jbd2_journal_commit_transaction
	journal_submit_data_buffers
 */


int add_pending_table(struct file *file)
{
	struct inode *inode = file->f_mapping->host;	
	struct ext4_inode_info *ei = EXT4_I(inode);

	ei->i_sstable = 1;
	ext4_dirty_inode(inode, 0);

	return 0;
}

static int do_check_commit(unsigned int fd)
{
	struct fd f = fdget(fd);
	int ret = -EBADF;

	if(f.file) {
		ret = add_pending_table(f.file); 
	}

	return ret;
}

SYSCALL_DEFINE1(check_commit, unsigned int, fd)
{
	return do_check_commit(fd);
}

static int do_is_committed(unsigned int fd)
{
	struct fd f = fdget(fd);
	struct inode *inode = f.file->f_mapping->host;
	struct super_block *sb = inode->i_sb;
	struct ext4_sb_info *sbi = EXT4_SB(sb);
	struct ext4_inode_info *ei;

	struct ct_node *cur = sbi->head;
	while(cur != sbi->tail) {
		printk("[is_committed] search : %p, %p\n", cur->vfs_inode, inode);
		if(cur->vfs_inode == inode) {
			return 1;
		}
		cur = cur->next;
	}

	return 0;
}

SYSCALL_DEFINE1(is_committed, unsigned int, fd)
{
	return do_is_committed(fd);
}
