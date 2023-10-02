#include <linux/syscalls.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/gfp.h>
#include <linux/fs.h>
#include <linux/file.h>

#include "ext4.h"
#include "ext4_jbd2.h"

struct inode_head *vt_alloc_inode_head(void)
{
	struct inode_head *ret;

	ret = kmalloc(sizeof(struct inode_head), GFP_ATOMIC);
	if(!ret) {
		/* do something */
	}
	return ret;
}

void vt_add_pending(struct inode_head **list, struct inode *inode, handle_t *handle)
{
	struct inode_head *ih;

	ih = vt_alloc_inode_head();

	ih->inode = inode;
	ih->ino = inode->i_ino;
	ih->tid = handle->h_transaction->t_tid;

	__ilist_add(list, ih);
}

void vt_move_entries(struct inode_head **committed_table, struct inode_head **pending_table)
{
	__ilist_move(committed_table, pending_table);
}

void vt_callback(struct super_block *sb, struct ext4_journal_cb_entry *jce, int rc)
{
	struct ext4_sb_info *sbi = EXT4_SB(sb);
	spin_lock(&sbi->s_vt_lock);
	vt_move_entries(&sbi->s_committed_table, &sbi->s_pending_table);
	spin_unlock(&sbi->s_vt_lock);
	sbi->s_callback_flag = 0;
	kfree(jce);
}

static int do_check_commit(unsigned int fd)
{
	struct fd f = fdget(fd);
	struct inode_head *ret = NULL;
	struct inode *inode = NULL;
	struct ext4_sb_info *sbi = NULL;

	if(f.file) {
		inode = f.file->f_mapping->host;
		sbi = EXT4_SB(inode->i_sb);

		ret = __ilist_find(&sbi->s_committed_table, inode->i_ino);
		if(ret) __ilist_del(&sbi->s_committed_table, ret);
		ret = __ilist_find(&sbi->s_pending_table, inode->i_ino);
		if(ret) __ilist_del(&sbi->s_pending_table, ret);

		EXT4_I(inode)->i_sstable = 1;
		EXT4_I(inode)->i_dirty = 0;
		fdput(f);
	}

	return 0;
}

SYSCALL_DEFINE1(check_commit, unsigned int, fd)
{
	return do_check_commit(fd);
}

int do_is_committed(unsigned int fd)
{
	struct fd f = fdget(fd);
	struct inode *inode = NULL;
	struct ext4_sb_info *sbi = NULL;
	struct inode_head *ret = NULL;

	if(f.file) {
		inode = f.file->f_mapping->host;
		sbi = EXT4_SB(inode->i_sb);
		ret = __ilist_find(&sbi->s_committed_table, inode->i_ino);
		fdput(f);
	}
	else {
		return 0;
	}

	if(ret == NULL) return 0;
	return 1;
}

SYSCALL_DEFINE1(is_committed, unsigned int, fd)
{
	return do_is_committed(fd);
}
