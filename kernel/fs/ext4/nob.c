#include <linux/fs.h>
#include <linux/jbd2.h>
#include <linux/err.h>
#include <linux/printk.h>
#include <linux/linkage.h>
#include <linux/syscalls.h>

#include "ext4.h"


int nob_track_template(handle_t *handle, struct inode *inode)
{
	bool update = false;
	struct ext4_inode_info *ei = EXT4_I(inode);
	struct ext4_sb_info *sbi = EXT4_SB(inode->i_sb);
	tid_t tid = 0;
	int ret;

	tid = handle->h_transaction->t_tid;

	mutex_lock(&ei->i_fc_lock);
	if (tid == ei->i_sync_tid) {
		ret = -EEXIST;
	} else {
		ext4_fc_reset_inode(inode);
		ei->i_sync_tid = tid;
		EXT4_I(inode)->i_fc_lblk_len = 0;
		ret = 0;
	}
	mutex_unlock(&ei->i_fc_lock);

	spin_lock(&sbi->s_fc_lock);
	if (list_empty(&EXT4_I(inode)->i_fc_list))
		list_add_tail(&EXT4_I(inode)->i_fc_list,
				(sbi->s_journal->j_flags & JBD2_FULL_COMMIT_ONGOING ||
				 sbi->s_journal->j_flags & JBD2_FAST_COMMIT_ONGOING) ?
				&sbi->s_fc_q[FC_Q_STAGING] :
				&sbi->s_fc_q[FC_Q_MAIN]);
	spin_unlock(&sbi->s_fc_lock);

	printk("[nob_track_template] inode : %p - %d\n", inode, inode->i_ino);
	return ret;
}

int nob_track_inode(struct inode *inode)
{
	int ret;
	handle_t *handle = ext4_journal_current_handle();

	if(!handle) {
		return;
	}

	ret = nob_track_template(ext4_journal_current_handle(), inode);

	return ret;
}

int nob_vfs_fsync(struct file *file) 
{
	struct inode *inode = file->f_mapping->host;
	
	return nob_track_inode(inode);
}

int _check_commit(unsigned int fd) 
{
	struct fd f = fdget(fd);
	int ret = -EBADF;

	if(f.file) {
		ret = nob_vfs_fsync(f.file);
	}

	return ret;
}

SYSCALL_DEFINE1(check_commit, int, fd)
{
	if(fd >= 0) {
		_check_commit(fd);
	}
	else {
		/* Error Handling */
	}
	return 0;
}
