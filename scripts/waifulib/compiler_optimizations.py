# encoding: utf-8
# compiler_optimizations.py -- main entry point for configuring C/C++ compilers
# Copyright (C) 2021 a1batross
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

try: from fwgslib import get_flags_by_type, get_flags_by_compiler
except: from waflib.extras.fwgslib import get_flags_by_type, get_flags_by_compiler
from waflib.Configure import conf
from waflib import Logs

'''
Flags can be overriden and new types can be added
by importing this as normal Python module

Example:
#!/usr/bin/env python
from waflib.extras import compiler_optimizations

compiler_optimizations.VALID_BUILD_TYPES += 'gottagofast'
compiler_optimizations.CFLAGS['gottagofast'] = {
	'gcc': ['-Ogentoo']
}
'''

VALID_BUILD_TYPES = ['fastnative', 'fast', 'humanrights', 'debug', 'sanitize', 'msan', 'ps3', 'none']

LINKFLAGS = {
	'common': {
		'msvc':  ['/DEBUG'], # always create PDB, doesn't affect result binaries
		'gcc':   ['-Wl,--no-undefined'],
		'owcc':  ['-Wl,option stack=512k']
	},
	'msan': {
		'clang': ['-fsanitize=memory', '-pthread'],
		'default': ['NO_MSAN_HERE']
	},
	'sanitize': {
		'clang': ['-fsanitize=undefined', '-fsanitize=address', '-pthread'],
		'gcc':   ['-fsanitize=undefined', '-fsanitize=address', '-pthread'],
		'msvc': ['/SAFESEH:NO']
	},
	'debug': {
		'msvc': ['/INCREMENTAL', '/SAFESEH:NO']
	}
}

CFLAGS = {
	'common': {
		# disable thread-safe local static initialization for C++11 code, as it cause crashes on Windows XP
		'msvc':    ['/D_USING_V110_SDK71_', '/FS', '/Zc:threadSafeInit-', '/MT', '/MP', '/Zc:__cplusplus'],
		'clang':   ['-g', '-gdwarf-2', '-fvisibility=hidden', '-fno-threadsafe-statics'],
		'gcc':     ['-g', '-fvisibility=hidden'],
		'owcc':	   ['-fno-short-enum', '-ffloat-store', '-g3']
	},
	'fast': {
		'msvc':    ['/O2', '/Oy', '/Zi'],
		'gcc': {
			'3':       ['-O3', '-fomit-frame-pointer'],
			'default': ['-Ofast', '-funsafe-math-optimizations', '-funsafe-loop-optimizations', '-fomit-frame-pointer', '-fno-semantic-interposition']
		},
		'clang':   ['-Ofast'],
		'default': ['-O3']
	},
	'fastnative': {
		'msvc':    ['/O2', '/Oy', '/Zi'],
		'gcc':     ['-Ofast', '-march=native', '-funsafe-math-optimizations', '-funsafe-loop-optimizations', '-fomit-frame-pointer', '-fno-semantic-interposition'],
		'clang':   ['-Ofast', '-march=native'],
		'default': ['-O3']
	},
	'humanrights': {
		'msvc':    ['/O2', '/Zi'],
		'owcc':    ['-O3', '-foptimize-sibling-calls', '-fomit-leaf-frame-pointer', '-fomit-frame-pointer', '-fschedule-insns', '-funsafe-math-optimizations', '-funroll-loops', '-frerun-optimizer', '-finline-functions', '-finline-limit=512', '-fguess-branch-probability', '-fno-strict-aliasing', '-floop-optimize'],
		'gcc':     ['-O3', '-fno-semantic-interposition'],
		'default': ['-O3']
	},
	'ps3': {
		'gcc':     ['-O2']
	},
	'debug': {
		'msvc':    ['/Od', '/ZI'],
		'owcc':    ['-O0', '-fno-omit-frame-pointer', '-funwind-tables', '-fno-omit-leaf-frame-pointer'],
		'default': ['-O0']
	},
	'msan': {
		'clang':   ['-O2', '-g', '-fno-omit-frame-pointer', '-fsanitize=memory', '-pthread'],
		'default': ['NO_MSAN_HERE']
	},
	'sanitize': {
		'msvc':    ['/Od', '/RTC1', '/Zi', '/fsanitize=address'],
		'gcc':     ['-O0', '-fsanitize=undefined', '-fsanitize=address', '-pthread'],
		'clang':   ['-O0', '-fsanitize=undefined', '-fsanitize=address', '-pthread'],
		'default': ['-O0']
	},
}

LTO_CFLAGS = {
	'msvc':  ['/GL'],
	'gcc':   ['-flto=auto'],
	'clang': ['-flto']
}

LTO_LINKFLAGS = {
	'msvc':  ['/LTCG'],
	'gcc':   ['-flto=auto'],
	'clang': ['-flto']
}

POLLY_CFLAGS = {
	'gcc':   ['-fgraphite-identity'],
	'clang': ['-mllvm', '-polly']
	# msvc sosat :(
}

def options(opt):
	grp = opt.add_option_group('Compiler optimization options')

	grp.add_option('-T', '--build-type', action='store', dest='BUILD_TYPE', default='humanrights',
		help = 'build type: debug, release or none(custom flags)')

	grp.add_option('--enable-lto', action = 'store_true', dest = 'LTO', default = False,
		help = 'enable Link Time Optimization if possible [default: %default]')

	grp.add_option('--enable-poly-opt', action = 'store_true', dest = 'POLLY', default = False,
		help = 'enable polyhedral optimization if possible [default: %default]')

def configure(conf):
	conf.start_msg('Build type')

	# legacy naming for default release build
	# https://chaos.social/@karolherbst/111340511652012860
	if conf.options.BUILD_TYPE == 'release':
		conf.options.BUILD_TYPE = 'humanrights'

	if not conf.options.BUILD_TYPE in VALID_BUILD_TYPES:
		conf.end_msg(conf.options.BUILD_TYPE, color='RED')
		conf.fatal('Invalid build type. Valid are: %s' % ', '.join(VALID_BUILD_TYPES))

	conf.end_msg(conf.options.BUILD_TYPE)

	conf.msg('LTO build', 'yes' if conf.options.LTO else 'no')
	conf.msg('PolyOpt build', 'yes' if conf.options.POLLY else 'no')

	# -march=native should not be used
	if conf.options.BUILD_TYPE.startswith('fast'):
		Logs.warn('WARNING: \'%s\' build type should not be used in release builds', conf.options.BUILD_TYPE)

	try:
		conf.env.CC_VERSION[0]
	except IndexError:
		conf.env.CC_VERSION = (0,)

@conf
def get_optimization_flags(conf):
	'''Returns a list of compile flags,
	depending on build type and options set by user

	NOTE: it doesn't filter out unsupported flags

	:returns: tuple of cflags and linkflags
	'''
	linkflags = conf.get_flags_by_type(LINKFLAGS, conf.options.BUILD_TYPE, conf.env.COMPILER_CC, conf.env.CC_VERSION[0])

	cflags = conf.get_flags_by_type(CFLAGS, conf.options.BUILD_TYPE, conf.env.COMPILER_CC, conf.env.CC_VERSION[0])

	if conf.options.LTO:
		linkflags+= conf.get_flags_by_compiler(LTO_LINKFLAGS, conf.env.COMPILER_CC)
		cflags   += conf.get_flags_by_compiler(LTO_CFLAGS, conf.env.COMPILER_CC)

	if conf.options.POLLY:
		cflags   += conf.get_flags_by_compiler(POLLY_CFLAGS, conf.env.COMPILER_CC)

	if conf.env.DEST_OS == 'nswitch' and conf.options.BUILD_TYPE == 'debug':
		# enable remote debugger
		cflags.append('-DNSWITCH_DEBUG')
	elif conf.env.DEST_OS == 'psvita':
		# this optimization is broken in vitasdk
		cflags.append('-fno-optimize-sibling-calls')
		# remove fvisibility to allow everything to be exported by default
		cflags.remove('-fvisibility=hidden')

	if conf.env.COMPILER_CC != 'msvc' and conf.env.COMPILER_CC != 'owcc':
		# HLSDK by default compiles with these options under Linux
		# no reason for us to not do the same

		# TODO: fix DEST_CPU in force 32 bit mode
		if conf.env.DEST_CPU == 'x86' or (conf.env.DEST_CPU == 'x86_64' and conf.env.DEST_SIZEOF_VOID_P == 4):
			cflags.append('-march=pentium-m')
			cflags.append('-mtune=core2')

	# on all compilers (except MSVC?) we need to copy CFLAGS to LINKFLAGS
	if conf.options.LTO and conf.env.COMPILER_CC != 'msvc':
		linkflags += cflags

	return cflags, linkflags
