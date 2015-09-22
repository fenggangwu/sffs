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


static void xfs_sffs_do_mount_original(xfs_mount_t *mp)
{
	printk("xfs_sffs: empty mount implementation.\n");
}

static void (*xfs_sffs_do_mount)(xfs_mount_t *) = 
	&xfs_sffs_do_mount_original;
EXPORT_SYMBOL(xfs_sffs_do_mount);

static void xfs_sffs_do_umount_original(xfs_mount_t *mp)
{
	printk("xfs_sffs: empty unmount implementation.\n");
}

static void (*xfs_sffs_do_umount)(xfs_mount_t *) = 
	&xfs_sffs_do_umount_original;
EXPORT_SYMBOL(xfs_sffs_do_umount);




void xfs_sffs_mount(xfs_mount_t *mp)
{
	printk("xfs_sffs: mount\n");
	xfs_sffs_do_mount(mp);
}
void xfs_sffs_umount(xfs_mount_t *mp)
{
	printk("xfs_sffs: unmount\n");
	xfs_sffs_do_umount(mp);
}





