#! /usr/bin/env python
# encoding: utf-8
# WARNING! Do not edit! https://waf.io/book/index.html#_obtaining_the_waf_file

#!/usr/bin/env python
# encoding: utf-8
# Thomas Nagy, 2006-2018 (ita)
# Ralf Habacker, 2006 (rh)
# Yinon Ehrlich, 2009

"""
snc detection.
"""

from waflib.Tools import ccroot, ar
from waflib.Configure import conf

@conf
def find_snc(conf):
	"""
	Find the program gcc, and if present, try to detect its version number
	"""
	conf.find_program(['ps3ppusnc'], var='SNC')
	conf.find_program(['ps3snarl'], var='ARL')
	conf.find_program(['ps3ppuld'], var='LD')

@conf
def snc_common_flags(conf):
	"""
	Common flags for gcc on nearly all platforms
	"""
	v = conf.env

	v.SNC_SRC_F            = []
	v.SNC_TGT_F            = ['-c', '-o']

	if not v.LINK_SNC:
		v.LINK_SNC = v.SNC

	v.SNCLNK_SRC_F         = []
	v.SNCLNK_TGT_F         = ['-o']
	v.SNCPATH_ST          = '-I%s'
	v.DEFINES_ST          = '-D%s'

	v.LIB_ST              = '-l%s' # template for adding libs
	v.LIBPATH_ST          = '-L%s' # template for adding libpaths
	v.STLIB_ST            = 'addmod %s'
	v.STLIBPATH_ST        = 'addmod %s'
	v.RPATH_ST            = '-Wl,-rpath,%s'

	v.SONAME_ST           = '-Wl,-h,%s'
	v.SHLIB_MARKER        = '-Wl,-Bdynamic'
	v.STLIB_MARKER        = '-Wl,-Bstatic'

	v.cprogram_PATTERN    = '%s.self'

	v.CFLAGS_cshlib       = []
	v.LINKFLAGS_cshlib    = []
	v.cshlib_ps3_PATTERN      = 'lib%s.sprx'

	v.LINKFLAGS_cstlib    = ['-Wl,-Bstatic']
	v.cstlib_ps3_PATTERN      = 'lib%s.a'


def configure(conf):
	"""
	Configuration for gcc
	"""
	conf.find_snc()
	conf.snc_common_flags()
	conf.cc_load_tools()
	conf.cc_add_flags()
	conf.link_add_flags()

