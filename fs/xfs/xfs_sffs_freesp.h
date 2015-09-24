#ifndef __XFS_SFFS_FREESP_H__
#define __XFS_SFFS_FREESP_H__

typedef struct xfs_sffs_superband_entry {
	xfs_agblock_t szonestart;
	xfs_agblock_t loghead;
	xfs_agblock_t logtail;
	xfs_agblock_t freecnt; /* free cnt = #clean block + #invalid block */
} xfs_sffs_superband_entry_t;

typedef struct xfs_sffs_freesp {
	xfs_sffs_superband_entry_t *table;
	unsigned long size;
} xfs_sffs_freesp_t;

int xfs_sffs_freesp_init(xfs_mount_t *mp, xfs_sffs_freesp_t *fsp);

#endif
