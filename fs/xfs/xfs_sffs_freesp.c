#include "xfs.h"
#include "xfs_fs.h"
#include "xfs_shared.h"
#include "xfs_format.h"
#include "xfs_log_format.h"
#include "xfs_trans_resv.h"
#include "xfs_bit.h"
#include "xfs_sb.h"
#include "xfs_mount.h"
#include "xfs_da_format.h"
#include "xfs_da_btree.h"
#include "xfs_inode.h"
#include "xfs_dir2.h"
#include "xfs_ialloc.h"
#include "xfs_alloc.h"
#include "xfs_rtalloc.h"
#include "xfs_bmap.h"
#include "xfs_trans.h"
#include "xfs_trans_priv.h"
#include "xfs_log.h"
#include "xfs_error.h"
#include "xfs_quota.h"
#include "xfs_fsops.h"
#include "xfs_trace.h"
#include "xfs_icache.h"
#include "xfs_sysfs.h"

#include "xfs_sffs_mount.h"

#include <linux/kernel.h>
#include <linux/module.h>

#include "xfs_sffs.h"
#include "xfs_sffs_freesp.h"

/* reference: xfs_alloc.c:xfs_alloc_read_agf() */
int xfs_sffs_freesp_init(xfs_mount_t *mp, xfs_sffs_freesp_t *freesp)
{

//	xfs_perag_t *pag;
	xfs_extlen_t longest = (xfs_extlen_t)0;
	xfs_buf_t	*bp;	/* agf buffer pointer */
	xfs_agf_t *agf;

	/* to find the first block of free extent, we have to calculate the 
	 * address were the internal log ends. internal log location is 
	 * specified by fsbock no. 
	 * since currently we only consider one ag. so the bsblock no. is the 
	 * agblock no.
	 */
	xfs_fsblock_t fs_free_start = (xfs_fsblock_t)0;
	xfs_agblock_t ag_free_start = (xfs_agblock_t)0; 
	unsigned long i;

//	struct xfs_buf **bpp = ;
	int error;
	
	error = xfs_read_agf(mp, NULL, 0, 0, &bp);

	if (error)
		return error;

	if (!bp)
		return 0;

	agf = XFS_BUF_TO_AGF(bp);

	if (bp)
		xfs_trans_brelse(NULL, bp);
	longest = be32_to_cpu(agf->agf_longest);
	printk("xfs_sffs: longest free extent %u blocks", longest);
	printk("xfs_sffs: free block num %u\n", 
	       be32_to_cpu(agf->agf_freeblks));

	printk("xfs_sffs: logstart %lu, logblocks %u\n", 
	       mp->m_sb.sb_logstart,
	       mp->m_sb.sb_logblocks);
	
	fs_free_start = mp->m_sb.sb_logstart + mp->m_sb.sb_logblocks;
	ag_free_start = XFS_FSB_TO_AGBNO(mp, fs_free_start);
	printk("xfs_sffs: free space starts from %u\n", ag_free_start);

	freesp->size = longest / XFS_SFFS_SZONE_BLKS;
	/* here the remainder blocks less than one superblock is discarded */
	freesp->table = vmalloc(sizeof(struct xfs_sffs_superband_entry) *
			     freesp->size);
	printk("xfs_sffs: vmalloc for freesp->table successfully\n");


	/*
	 *  | |x|d|d|x|d| | | ... |
	 *     ^         ^
	 *     |         | 
	 *   tail       head  ->
	 *  
	 *  tail      first non-clean block
	 *  head      first clean block
	 *  |d|       allocated block
	 *  |x|       invalid block (stale data block)
	 *  | |       clean block
	 *  freecnt   #clean block + #invalid block 
	 * 
	 * Note that both the log head and tail pointer stores AGBNO,
	 * grows towards the right, and can wrap around from the start.
	 * Also, we avoid the case to happen where the head pointer wraps
	 * around and grows up to the same position of the tail pointer. 
	 * In this case  we cannot tell whether the superband is full or 
	 * empty. So we set the upper limit of head pointer to be tail - 1 
	 * (in a wrapping sense). By sacrificing one block, we can prevent 
	 * the ambigous case from happening.
	 */

	printk("%7s%11s%11s%11s%11s\n", "idx", "szonestart", "loghead", 
	       "logtail", "freecnt");
	for (i = 0; i < freesp->size; i++) {
		freesp->table[i].szonestart = ag_free_start + XFS_SFFS_SZONE_BLKS * i;
		freesp->table[i].loghead = freesp->table[i].szonestart;
		freesp->table[i].logtail = freesp->table[i].szonestart;
		/* leave at least one guard block to prevent head pointer 
		   to hit tail poitner.*/
		freesp->table[i].freecnt = XFS_SFFS_SZONE_BLKS - 1; 
		/*
		printk("%7d%19u%19u%19u\n%7s%19x%19x%19x\n%7s%19lx%19lx%19lx\n", 
		       i,
		       freesp->table[i].loghead,
		       freesp->table[i].logtail,
		       freesp->table[i].freecnt,
		       "",
		       freesp->table[i].loghead,
		       freesp->table[i].logtail,
		       freesp->table[i].freecnt,
		       "",
		       &freesp->table[i].loghead,
		       &freesp->table[i].logtail,
		       &freesp->table[i].freecnt);

		*/

		printk("%7lu%11u%11u%11u%11u\n",
		       i,
		       freesp->table[i].szonestart,
		       freesp->table[i].loghead,
		       freesp->table[i].logtail,
		       freesp->table[i].freecnt);
	}

	
	
	printk("xfs_sffs: init xfs_sffs_freesp....\n");
	printk("xfs_sffs: XFS_SFFS_ZONE_BLKS:%d\n", XFS_SFFS_ZONE_BLKS);
	printk("xfs_sffs: XFS_SFFS_SZONE_BLKS:%d\n", XFS_SFFS_SZONE_BLKS);
	printk("xfs_sffs: XFS_SFFS_SZONE_INIT_BLK_QUOTA:%d\n", XFS_SFFS_SZONE_INIT_BLK_QUOTA);
	printk("xfs_sffs: %lu table entries created\n", freesp->size);
	

	return 0;
}
