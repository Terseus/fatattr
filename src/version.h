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

#ifndef __VERSION_H__
#define __VERSION_H__

#define VERSION_STRINGIZE_(x) #x
#define VERSION_STRINGIZE(x) VERSION_STRINGIZE_(x)
#define PROGRAM_NAME        "fatattr"
#define VERSION_PRIMARY     1
#define VERSION_SECONDARY   0
#define VERSION_PATCH       0
#define VERSION_STRING  VERSION_STRINGIZE(VERSION_PRIMARY) "." \
                        VERSION_STRINGIZE(VERSION_SECONDARY) "." \
                        VERSION_STRINGIZE(VERSION_PATCH)

#endif /* __VERSION_H__ */
