#ifndef __XFS_SFFS_H__
#define __XFS_SFFS_H__

/* 4KiB per block */
#define XFS_SFFS_BLK_SHIFT                                                  12
#define XFS_SFFS_BLK_SZ                              (1 << XFS_SFFS_BLK_SHIFT)

/* 256MB per zone */
#define XFS_SFFS_ZONE_BLK_SHIFT                      (28 - XFS_SFFS_BLK_SHIFT)
#define XFS_SFFS_ZONE_BLKS                      (1 << XFS_SFFS_ZONE_BLK_SHIFT)

/*relative shift from superzone (SZONE) to zone.
  1 superzone = 2G (32bit address) = 8 band, by default */
#define XFS_SFFS_SZONE_ZONE_SHIFT                                            3
#define XFS_SFFS_SZONE_BLKS  (XFS_SFFS_ZONE_BLKS << XFS_SFFS_SZONE_ZONE_SHIFT)

#define XFS_SFFS_AGBNO_TO_SZONE(agbno)    \
	(agbno >> (XFS_SFFS_SZONE_ZONE_SHIFT + XFS_SFFS_ZONE_BLK_SHIFT))

#endif /* __XFS_SFFS_H__ */
