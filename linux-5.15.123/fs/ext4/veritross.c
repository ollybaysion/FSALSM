#include <linux/syscalls.h>
#include <linux/kernel.h>
#include <linux/fs.h>

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

	- fs/ext4/inode.c
	ext4_dirty_inode
	ext4_mark_inode_dirty
	__ext4_mark_inode_dirty
	ext4_mark_iloc_dirty

	- fs/ext4/fast_commit.c
	ext4_fc_track_inode
	ext4_fc_track_template
	list_add_tail(inode, sb->s_fc_q)

	file->f_op->fsync
	fs/ext4/file.c
	struct file_operations ext4_file_operations.fsync = ext4_sync_file

	- fs/ext4/fsync.c
	ext4_sync_file
	ext4_fsync_journal
	ext4_fc_commit
	ext4_fc_perform_commit
	ext4_fc_submit_inode_data_all
	ext4_fc_wait_inode_data_all
 */


int add_pending_table(struct file *file)
{
	struct inode *inode = file->f_mapping->host;	
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

