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

#include "dosfs.h"
#include "bool.h"
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/ioctl.h>

#define ERRMSG_MAX 1025
#define DIRENT_SIZE 2

enum {
    ENOERR = 0,
    EOPEN,
    EIOCTL_GET_ATTRIBUTES,
    EIOCTL_SET_ATTRIBUTES,
    EIOCTL_READDIR_BOTH,
    EBUFFER
};

typedef enum {
    MADD,
    MREMOVE
} tDosfsModifyType;

static char errmsg[ERRMSG_MAX] = {0};

/**
 * Modify the attributes of a file descriptor, 'modifyType' specifies if the
 * attributes are added (MADD) or removed (MREMOVE).
 * Returns 0 on success, !0 if an error happens.
 */
int dosfsModifyAttributes(int fd, uint32_t attrs,
                          tDosfsModifyType modifyType);


int dosfsModifyAttributes(int fd, uint32_t attrs,
                          tDosfsModifyType modifyType)
{
	assert(fd != -1);
	uint32_t currentAttrs = 0;
	int ioctlRet = ioctl(fd, FAT_IOCTL_GET_ATTRIBUTES, &currentAttrs);
	if (ioctlRet < 0) {
		return EIOCTL_GET_ATTRIBUTES;
	}
	uint32_t newAttrs = 0;
	switch (modifyType) {
	case MADD:
		newAttrs = currentAttrs | attrs;
		break;
	case MREMOVE:
		newAttrs = (~attrs) & currentAttrs;
		break;
	}
	ioctlRet = ioctl(fd, FAT_IOCTL_SET_ATTRIBUTES, &newAttrs);
	if (ioctlRet < 0) {
		return EIOCTL_SET_ATTRIBUTES;
	}
	return ENOERR;
}


const char *dosfsGetError(int err)
{
	switch (err) {
	case ENOERR:
		snprintf(errmsg, ERRMSG_MAX,
		         "No error occurred");
		break;
	case EOPEN:
		snprintf(errmsg, ERRMSG_MAX,
		         "Error opening file: %s",
		         strerror(errno));
		break;
	case EIOCTL_GET_ATTRIBUTES:
		snprintf(errmsg, ERRMSG_MAX,
		         "Error in ioctl call 'FAT_IOCTL_GET_ATTRIBUTES': %s",
		         strerror(errno));
		break;
	case EIOCTL_SET_ATTRIBUTES:
		snprintf(errmsg, ERRMSG_MAX,
		         "Error in ioctl call 'FAT_IOCTL_SET_ATTRIBUTES': %s",
		         strerror(errno));
		break;
	case EIOCTL_READDIR_BOTH:
		snprintf(errmsg, ERRMSG_MAX,
		         "Error in ioctl call 'VFAT_IOCTL_READDIR_BOTH': %s",
		         strerror(errno));
		break;
	case EBUFFER:
		snprintf(errmsg, ERRMSG_MAX,
		         "The entry name is bigger than the read buffer");
		break;
	default:
		snprintf(errmsg, ERRMSG_MAX,
		         "Unknown error");
	}
	return errmsg;
}

int dosfsOpen(const char *file, int *fd)
{
	assert(file != NULL);
	assert(fd != NULL);
	/* O_RDONLY works with files and directories (write doesn't) and let us
	   modify FAT attributes. */
	*fd = open(file, O_RDONLY);
	if (*fd == -1) {
		return EOPEN;
	}
	return ENOERR;
}

int dosfsClose(int fd)
{
	assert(fd != -1);
	close(fd);
	return ENOERR;
}

int dosfsGetAttributes(int fd, uint32_t *attrs)
{
	assert(attrs != NULL);
	int ioctlRet = ioctl(fd, FAT_IOCTL_GET_ATTRIBUTES, attrs);
	if (ioctlRet < 0) {
		return EIOCTL_GET_ATTRIBUTES;
	}
	return ENOERR;
}

int dosfsAddAttributes(int fd, uint32_t attrs)
{
	return dosfsModifyAttributes(fd, attrs, MADD);
}

int dosfsRemoveAttributes(int fd, uint32_t attrs)
{
	return dosfsModifyAttributes(fd, attrs, MREMOVE);
}

int dosfsReadDir(int fd, char *entry, size_t pathSize)
{
	assert(entry != NULL);
	assert(fd != -1);
	/* VFAT_IOCTL_READDIR_BOTH expects 2 __fat_dirent objects, one for the
	   short name entry an one for the long name entry. */
	struct __fat_dirent dirEnt[DIRENT_SIZE];
	int ioctlRet = ioctl(fd, VFAT_IOCTL_READDIR_BOTH, dirEnt);
	if (ioctlRet < 0) {
		close(fd);
		return EIOCTL_READDIR_BOTH;
	} else if (ioctlRet == 0) {
		memset(entry, '\0', pathSize);
		return ENOERR;
	}
	/* If the *real* file name have 8 characters or less, the long name entry
	   will have d_name empty. */
	int dirEntRealName = strlen(dirEnt[1].d_name) == 0 ? 0 : 1;
	if (strlen(dirEnt[dirEntRealName].d_name) >= pathSize) {
		return EBUFFER;
	}
	strncpy(entry, dirEnt[dirEntRealName].d_name, pathSize - 1);
	return ENOERR;
}

