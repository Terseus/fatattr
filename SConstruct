'''
Copyright 2013 David Caro Martinez

This file is part of fatattr.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
'''

import os


def sourceList(objdir, names):
    return [os.path.join(objdir, x) for x in names]


Help("""
Type: 'scons' to build the main program.

Accepted parameters:
	build=<debug|release>
		Build type (default: release)
	printenv=<0|1|2>
		Prints the build environment - for debugging purposes
""")

V_BUILD_TYPE = ARGUMENTS.get('build', 'release')
V_PRINTENV = int(ARGUMENTS.get('printenv', 0))
V_SRC_DIR = 'src/'
V_INC_DIR = V_SRC_DIR
V_BUILD_DIR = 'build/'
V_MAIN_X = 'bin/fatattr'
V_CFLAGS_BASE = '-pedantic -std=c11'
V_CFLAGS_DEBUG = '-Weverything -O0 -g -DDEBUG'
V_CFLAGS_RELEASE = '-O2'
V_CFLAGS = ''
V_MAIN_C = sourceList(V_BUILD_DIR, ['main.c'])
V_DOSFS_C = sourceList(V_BUILD_DIR, ['dosfs.c'])

if V_BUILD_TYPE == 'release':
	V_CFLAGS = '%s %s' % (V_CFLAGS_BASE, V_CFLAGS_RELEASE)
elif V_BUILD_TYPE == 'debug':
	V_CFLAGS = '%s %s' % (V_CFLAGS_BASE, V_CFLAGS_DEBUG)
else:
	print("Invalid build type '%s'" % (V_BUILD_TYPE))
	Exit(1)

env = Environment(CPPPATH = V_INC_DIR,
                  CC = 'clang',
                  TERM = os.environ['TERM'],
                  CFLAGS = V_CFLAGS)
env.VariantDir(V_BUILD_DIR, V_SRC_DIR, duplicate=0)
if V_PRINTENV > 0:
    print("Printing environment...")
    varlist = locals()
    name = ''
    for name in varlist:
        if name.startswith('V_'):
            print("%s => '%s'" % (name, varlist[name]))
    if V_PRINTENV > 1:
        print("Dumping SCons environment...")
        print env.Dump()
    exit(0)

dosfs_o = env.Object(V_DOSFS_C)
main_o = env.Object(V_MAIN_C)
main_x = env.Program(V_MAIN_X, main_o + dosfs_o)
