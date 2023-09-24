#include <linux/syscalls.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/gfp.h>
#include <linux/fs.h>

#include "ext4.h"
#include "ext4_jbd2.h"

static inline void
__ilist_add(struct inode_head **list, struct inode_head *ih) 
{
	if(!*list) {
		ih->next = ih->prev = ih;
		*list = ih;
	} else {
		struct inode_head *first = *list, *last = first->prev;
		ih->prev = last;
		ih->next = first;
		last->next = first->prev = ih;
	}
}

static inline void
__ilist_del(struct inode_head **list, struct inode_head *ih)
{
	if(*list == ih) {
		*list = ih->next;
		if(*list == ih)
			*list = NULL;
	}
	ih->prev->next = ih->next;
	ih->next->prev = ih->prev;
}

static inline void
__ilist_move(struct inode_head **list1, struct inode_head **list2)
{
	if(*list1 == NULL) {
		*list1 = *list2;
	} else if (*list2 != NULL) {
		struct inode_head *first1 = *list1, *last1 = first1->prev;
		struct inode_head *first2 = *list2, *last2 = first2->prev;
		first2->prev = last1;
		last2->next = first1;
		last1->next = first2;
		first1->prev = last2;
	}
	*list2 = NULL;
}

static inline int
__ilist_find(struct inode_head **list, unsigned long t_ino)
{
	if(*list) {
		struct inode_head *first = *list, *cur = first;
		while(cur->next != first) {
			printk("[veritross] cur : %ld, finding : %d\n", cur->ino, t_ino);
			if(cur->ino == t_ino) {
				return 1;
			}
			cur = cur->next;
		}
	}
	return 0;
}

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

	if(f.file) {
		struct inode *inode = f.file->f_mapping->host;
		EXT4_I(inode)->i_sstable = 1;
//		ext4_dirty_inode(inode, 0);
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
	struct ext4_sb_info *sbi = EXT4_SB(inode->i_sb);

	int ret = __ilist_find(&sbi->s_committed_table, inode->i_ino);

	return ret;
}

SYSCALL_DEFINE1(is_committed, unsigned int, fd)
{
	return do_is_committed(fd);
}
