#ifndef __XFS_SFFS_H__
#define __XFS_SFFS_H__

/* 4KiB per block */
#define XFS_SFFS_BLK_SHIFT                                                  12
#define XFS_SFFS_BLK_SZ                              (1 << XFS_SFFS_BLK_SHIFT)

/* 1MB per band */
#define XFS_SFFS_BAND_BLK_SHIFT                      (20 - XFS_SFFS_BLK_SHIFT)
#define XFS_SFFS_BAND_BLKS                      (1 << XFS_SFFS_BAND_BLK_SHIFT)

/*relative shift from superband (SBAND) to band.
  1 superband = 256 band, by default */
#define XFS_SFFS_SBAND_BAND_SHIFT                                            8
#define XFS_SFFS_SBAND_BLKS  (XFS_SFFS_BAND_BLKS << XFS_SFFS_SBAND_BAND_SHIFT)

#define XFS_SFFS_AGBNO_TO_SBAND(agbno)    \
	(agbno >> (XFS_SFFS_SBAND_BAND_SHIFT + XFS_SFFS_BAND_BLK_SHIFT))

#endif /* __XFS_SFFS_H__ */
