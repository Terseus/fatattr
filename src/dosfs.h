/**
 * Copyright 2013 David Caro Martinez
 *
 * This file is part of fatattr.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __DOSFS_H__
#define __DOSFS_H__

#include <linux/msdos_fs.h>
#include <stdint.h>
#include <stdlib.h>

/* Redefinition of attribute macros, so we don't depend on msdos_fs.h macros. */
enum {
	DOSFS_ATTR_RO = ATTR_RO,
	DOSFS_ATTR_HIDDEN = ATTR_HIDDEN,
	DOSFS_ATTR_SYS = ATTR_SYS,
	DOSFS_ATTR_VOLUME = ATTR_VOLUME,
	DOSFS_ATTR_DIR = ATTR_DIR,
	DOSFS_ATTR_ARCH = ATTR_ARCH
};
/* Attributes checks for lazyness. */
#define DOSFS_HAS_ATTR(x, a)        ((x) & (a))
#define DOSFS_HAS_ATTR_RO(x)        DOSFS_HAS_ATTR(x, DOSFS_ATTR_RO)
#define DOSFS_HAS_ATTR_HIDDEN(x)    DOSFS_HAS_ATTR(x, DOSFS_ATTR_HIDDEN)
#define DOSFS_HAS_ATTR_SYS(x)       DOSFS_HAS_ATTR(x, DOSFS_ATTR_SYS)
#define DOSFS_HAS_ATTR_VOLUME(x)    DOSFS_HAS_ATTR(x, DOSFS_ATTR_VOLUME)
#define DOSFS_HAS_ATTR_DIR(x)       DOSFS_HAS_ATTR(x, DOSFS_ATTR_DIR)
#define DOSFS_HAS_ATTR_ARCH(x)      DOSFS_HAS_ATTR(x, DOSFS_ATTR_ARCH)


/**
 * Returns a descriptive message associated with an error code.
 */
const char *dosfsGetError(int err);
/**
 * Open a file and save its file descriptor in fd.
 * Returns 0 on success, !0 if an error happens.
 */
int dosfsOpen(const char *file, int *fd);
/**
 * Close a file descriptor.
 * Returns 0 on success, !0 if an error happens.
 */
int dosfsClose(int fd);
/**
 * Get the FAT attributes from a file descriptor and save them in 'attrs'.
 * Returns 0 on success, !0 if an error happens.
 */
int dosfsGetAttributes(int fd, uint32_t *attrs);
/**
 * Add FAT attributes to a file descriptor.
 * Returns 0 on success, !0 if an error happens.
 */
int dosfsAddAttributes(int fd, uint32_t attrs);
/**
 * Remove FAT attributes from a file descriptor.
 * Returns 0 on success, !0 if an error happens.
 */
int dosfsRemoveAttributes(int fd, uint32_t attrs);
/**
 * Read the next entry of the directory associated with a file descriptor.
 * Write the long name entry in 'entry', writing at most 'pathSize' - 1
 * characters.
 * Returns 0 on success, !0 if an error happens or if the complete long name
 * doesn't fit in 'entry'.
 */
int dosfsReadDir(int fd, char *entry, size_t pathSize);

#endif /* __DOSFS_H__ */
