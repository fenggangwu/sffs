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

static void (*xfs_sffs_mount_origin)(xfs_mount_t *);
extern void (*xfs_sffs_do_mount)(xfs_mount_t *);

static void (*xfs_sffs_umount_origin)(xfs_mount_t *);
extern void (*xfs_sffs_do_umount)(xfs_mount_t *);

static void (*xfs_alloc_ag_vextent_exact_origin)(xfs_alloc_arg_t *);
extern void (*xfs_alloc_ag_vextent_exact)(xfs_alloc_arg_t *);

static void (*xfs_alloc_ag_vextent_near_origin)(xfs_alloc_arg_t *);
extern void (*xfs_alloc_ag_vextent_near)(xfs_alloc_arg_t *);

static void (*xfs_alloc_ag_vextent_size_origin)(xfs_alloc_arg_t *);
extern void (*xfs_alloc_ag_vextent_size)(xfs_alloc_arg_t *);

static int (*xfs_free_extent_origin)(
			  xfs_trans_t *, xfs_fsblock_t, xfs_extlen_t);
extern int (*xfs_free_extent_worker)(
			  xfs_trans_t *, xfs_fsblock_t, xfs_extlen_t);

extern int xfs_alloc_ag_vextent_exact_original(xfs_alloc_arg_t *);
extern int xfs_alloc_ag_vextent_near_original(xfs_alloc_arg_t *);
extern int xfs_alloc_ag_vextent_size_original(xfs_alloc_arg_t *);

extern void xfs_alloc_fix_len(xfs_alloc_arg_t *);


/*                                                                              * Helper function for allocting from the free space table loghead pointer.     * In sffs, all three AG alloc type will be served by this helper function.     */
STATIC int
xfs_sffs_alloc_ag_vextent_smr(
			 xfs_alloc_arg_t *args)
{
	int i;
	int longest = 0; /* the longest available extent in the super band */
	xfs_agblock_t head; /* loghead pointer */
	xfs_agblock_t tail; /* logend pointer */
	xfs_agblock_t szone_start; /* startbno for the superzone */
	xfs_agblock_t szone_end;

	int sbandcase;
	xfs_mount_t *mp = args->mp;
	
	printk("\n");
	printk("xfs_sffs: type=%d\n", args->type);
	printk("%10sfsbno=%lu agno=%u agbno=%u\n", 
	       "", args->fsbno, args->agno, args->agbno);
	printk("%10sminlen=%u maxlen=%u minleft=%u total=%u len=%u\n", 
	       "", args->maxlen, args->minlen, args->minleft, args->total,
	       args->len);
	//	printk("%10slen=%x, &args->len=%x\n", "", args->len, &args->len);
	printk("%10smod=%u prod=%u\n", 
	       "", args->mod, args->prod);
	printk("%10salignment=%u minalignslop=%u\n", 
	       "", args->alignment, args->minalignslop);
	printk("%10stype=%d otype=%d\n", 
	       "", args->type, args->otype);
	printk("%10swasdel=%d wasfromfl=%d isfl=%d\n", 
	       "", args->wasdel, args->wasfromfl, args->isfl);

	printk("%10suserdata=%d firstblock=%lu\n", 
	       "", args->userdata, args->firstblock);

	switch (args->type) {
        case XFS_ALLOCTYPE_THIS_BNO:   /* for exact bno */
		ASSERT(XFS_SFFS_AGBNO_TO_SBAND(args->agbno) < 
		       mp->m_freesp->size);
		if (args->agbno != 
		    mp->m_freesp->table[XFS_SFFS_AGBNO_TO_SZONE(args->agbno)].
		    loghead) {
			goto outnoblock;
		}
		/* fall through */
	case XFS_ALLOCTYPE_THIS_AG:    /* by size */
        case XFS_ALLOCTYPE_NEAR_BNO:   /* near bno */
		/* all these above case will end up in a sequential allocation
		 * in the log head pointer.
		 * current version of allocation algorithm will just find the
		 * first availble SBAND and allocat the most space that can
		 * be support by the remaining space.
		 */

		for (i = 0; i < mp->m_freesp->size; i++) {
			szone_start = mp->m_freesp->table[i].szonestart;
			szone_end = szone_start + XFS_SFFS_SZONE_BLKS - 1;
			head = mp->m_freesp->table[i].loghead;
			tail = mp->m_freesp->table[i].logtail; 
			/*
			printk("xfs_sffs: iter\n%sszone[%d] start=%u end=%u head=%u tail=%u freecnt=%u\n",
			       "", i, szone_start, szone_end, 		     
			       mp->m_freesp->table[i].loghead,
			       mp->m_freesp->table[i].logtail,
			       mp->m_freesp->table[i].freecnt);
			*/
			/*
			printk("%10stable[i].loghead=%x, &=%lx\n", 
			       "", mp->m_freesp->table[i].loghead,
			       &mp->m_freesp->table[i].loghead);

			printk("%10stable[i].logtail=%x, &=%lx\n", 
			       "", mp->m_freesp->table[i].logtail,
			       &mp->m_freesp->table[i].logtail);

			printk("%10stable[i].freecnt=%x, &=%lx\n", 
			       "", mp->m_freesp->table[i].freecnt,
			       &mp->m_freesp->table[i].freecnt);


			printk("%10stable[i].szonestart=%x, &=%lx\n", 
			       "", mp->m_freesp->table[i].szonestart,
			       &mp->m_freesp->table[i].szonestart);

			printk("%10shead=%x, &=%lx\n", 
			       "", head, &head);

			printk("%10stail=%x, &=%lx\n", 
			       "", tail, &tail);
			*/

			if (head >= tail) {
				if (tail != szone_start) {
					/*
					 * case 1:
					 *          longest contiguous extent
					 *                |<------->|
					 *  | | |x|d|d|x|d| | | ... |
					 *       ^         ^
					 *       |         | 
					 *     tail       head  ->
					 *
					 * longest = [head, END]
					 * note that we can't directly allocate
					 * the clean block by wrapping around
					 * from the start, because it is not a 
					 * contigous data extent.
					 */
					sbandcase = 1;
					longest = szone_end - head + 1;
				} else /* tail == 0 */ {
					/*
					 * case 2:
					 *          longest contiguous extent
					 *              |<------->|
					 *  |d|x|x|d|d|x| | | |...| |
					 *   ^           ^
					 *   |           | 
					 *  tail       head  ->
					 *
					 * longest = [head, END - 1]
					 * because head pointer cannot hit tail
					 */
					sbandcase = 2;
					longest = szone_end - head;
				}
			} else {/* head < tail */

				/*
				 * case 3:
				 *     longest contiguous extent
				 *        |<----->|
				 *  |d|x|d| | | | | |d|d|x|d| ... |
				 *         ^         ^
				 *         |         | 
				 *        head       tail  ->
				 * longest = [head, tail - 2]
				 * because head pointer cannot hit tail
				 */
				sbandcase = 3;
				longest = tail - head - 1;
			}

			printk("xfs_sffs: case=%d longest=%u\n", 
			       sbandcase, longest);
			/* give up if longest == 0 or less then min request */
			if (!longest || longest < args->minlen) {
				printk("xfs_sffs: szone=%d, longest=%u, args->minlen=%u, continue search for next super zone\n",
				       i, longest, args->minlen);
				continue;
			}
			/*
			printk("xfs_sffs: before\n%ssb[%d] start=%u end=%u head=%u tail=%u freecnt=%u\n",
			       "", i, szone_start, szone_end, 		     
			       mp->m_freesp->table[i].loghead,
			       mp->m_freesp->table[i].logtail,
			       mp->m_freesp->table[i].freecnt);
			*/

			args->agbno = head;
			/* len should be the smaller between the two */
			args->len = longest < args->maxlen? 
				longest : args->maxlen;
			
			/* fix the length according to mod and prod if given */
			xfs_alloc_fix_len(args);
			args->wasfromfl = 0;

			/* fix up the free space table */
			mp->m_freesp->table[i].loghead += args->len;
			switch (sbandcase) {
			case 1:
				ASSERT(mp->m_freesp_table[i].loghead 
				       <= szone_end + 1);
				if (mp->m_freesp->table[i].loghead == 
				    szone_end + 1)
					mp->m_freesp->table[i].loghead = 
						szone_start;
				break;
			case 2:
				ASSERT(mp->m_freesp_table[i].loghead 
				       <= szone_end);
				break;
			case 3:
			        ASSERT(mp->m_freesp->table[i].loghead <= 
				       tail - 1);
				break;
			default:
				ASSERT(0);
			}
			mp->m_freesp->table[i].freecnt -= args->len;
			printk("xfs_sffs: szone=%d [%u %u] head=%u tail=%u freecnt=%u\n",
			       i, szone_start, szone_end, 		     
			       mp->m_freesp->table[i].loghead,
			       mp->m_freesp->table[i].logtail,
			       mp->m_freesp->table[i].freecnt);

			goto outsuccess;
		}
		break;
        default:
                ASSERT(0);
                /* NOTREACHED */
	}

 outnoblock:
	printk("xfs_sffs: alloc fails\n");
	args->agbno = NULLAGBLOCK;
	return 0;

 outsuccess:
	printk("xfs_sffs: alloc success bno=%u, len=%u\n", 
	       args->agbno, args->len);
	return 0;
}

int xfs_free_extent_impl(xfs_trans_t *tp, xfs_fsblock_t bno, xfs_extlen_t len)
{
	printk("xfs_sffs: free extent (bno, len) = (%lu, %lu)\n", bno, len);
	printk("          now do nothing. Postpone the implmentation for this fuction until GC\n");
	return 0;
}

/*
 * Now for simplicity of the problem, the following
 * shoud be checked before mounting:
 * There is only one AG currently.
 */
static inline int /* 1 if empty & clean fs; 0 if not */
__xfs_sffs_check(xfs_mount_t *mp)
{
	xfs_sffs_freesp_init(mp, mp->m_freesp);
	return 0;
}


static void xfs_sffs_mount_impl(xfs_mount_t *mp)
{
	printk("xfs_sffs: mount implemetation\n");
//	printk("xfs_sffs: ")
	mp->m_freesp = vmalloc(sizeof(struct xfs_sffs_freesp));
	printk("xfs_sffs: vmalloc for struct freesp successfully\n");
	mp->m_freesp->table = NULL;
	mp->m_freesp->size = 0;
	__xfs_sffs_check(mp);
}


static void xfs_sffs_umount_impl(xfs_mount_t *mp)
{
	printk("xfs_sffs: unmount implemetation\n");
	if (mp->m_freesp) {
		if (mp->m_freesp->table) {
			printk("xfs_sffs: freeing freesp table..\n");
			vfree(mp->m_freesp->table);
		}
		vfree(mp->m_freesp);
	}
}

int init_module(void)
{
	printk("xfs_sffs: init_module\n");
	
	xfs_sffs_mount_origin = xfs_sffs_do_mount;
	xfs_sffs_do_mount = &xfs_sffs_mount_impl;
	
	xfs_sffs_umount_origin = xfs_sffs_do_umount;
	xfs_sffs_do_umount = &xfs_sffs_umount_impl;

	xfs_alloc_ag_vextent_exact_origin = xfs_alloc_ag_vextent_exact;
	xfs_alloc_ag_vextent_exact = &xfs_sffs_alloc_ag_vextent_smr;

	xfs_alloc_ag_vextent_near_origin = xfs_alloc_ag_vextent_near;
	xfs_alloc_ag_vextent_near = &xfs_sffs_alloc_ag_vextent_smr;

	xfs_alloc_ag_vextent_size_origin = xfs_alloc_ag_vextent_size;
	xfs_alloc_ag_vextent_size = &xfs_sffs_alloc_ag_vextent_smr;

	xfs_free_extent_origin = xfs_free_extent_worker;
	xfs_free_extent_worker = &xfs_free_extent_impl;
	return 0;
}


void cleanup_module(void)
{
	printk("xfs_sffs: cleanup_module\n");
	xfs_sffs_do_mount = xfs_sffs_mount_origin;
	xfs_sffs_do_umount = xfs_sffs_umount_origin;

	xfs_alloc_ag_vextent_exact = xfs_alloc_ag_vextent_exact_origin;
	xfs_alloc_ag_vextent_near = xfs_alloc_ag_vextent_near_origin;
	xfs_alloc_ag_vextent_size = xfs_alloc_ag_vextent_size_origin;
	
	xfs_free_extent_worker = xfs_free_extent_origin;
}


MODULE_LICENSE("GPL");
