# encoding: utf-8
# ps3.py -- PS3 task
# Copyright (C) 2023 fgsfds
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

from waflib.Tools import ccroot, c, cxx
from waflib.Tools.ccroot import link_task
from waflib import *

def add_source_file(ctx, nodes, f):
	if f:
		if isinstance(f, str):
			node = ctx.path.make_node(f)
		elif isinstance(f, Node.Node):
			node = f
		nodes += [node]
		return node
	return None

def configure(conf):
	conf.find_program('make_fself', var='MAKE_FSELF')

class mkfself(Task.Task):
	color = 'CYAN'
	run_str = '${MAKE_FSELF} ${ELFFILE} ${TGT}'


class cprogram_ps3(c.cprogram):
	run_str = '${LINK_CC} ${LINKFLAGS} -Wl,-oformat=fself ${LIBPATH_ST:LIBPATH} ${LIB_ST:LIB} ${STLIBPATH_ST:STLIBPATH} ${STLIB_ST:STLIB} -Wl,--start-group ${CCLNK_SRC_F}${SRC} ${LIB} -Wl,--end-group ${CCLNK_TGT_F} ${TGT[0].abspath()}'

class cxxprogram_ps3(cxx.cxxprogram):
	run_str = '${LINK_CXX} ${LINKFLAGS} -Wl,-oformat=fself ${LIBPATH_ST:LIBPATH} ${LIB_ST:LIB} ${STLIBPATH_ST:STLIBPATH} ${STLIB_ST:STLIB} -Wl,--start-group ${CXXLNK_SRC_F}${SRC} ${LIB} -Wl,--end-group ${CXXLNK_TGT_F} ${TGT[0].abspath()}'

class cxx_ps3(cxx.cxx):
	run_str = '${CXX} ${ARCH_ST:ARCH} ${CXXFLAGS} ${FRAMEWORKPATH_ST:FRAMEWORKPATH} ${CPPPATH_ST:INCPATHS} ${DEFINES_ST:DEFINES} ${CXX_SRC_F}${SRC} ${CXX_TGT_F}${TGT[0].abspath()} ${CPPFLAGS}'

class c_ps3(c.c):
	run_str = '${CC} ${ARCH_ST:ARCH} ${CFLAGS} ${FRAMEWORKPATH_ST:FRAMEWORKPATH} ${CPPPATH_ST:INCPATHS} ${DEFINES_ST:DEFINES} ${CC_SRC_F}${SRC} ${CC_TGT_F}${TGT[0].abspath()} ${CPPFLAGS}'

class cxxshlib_ps3(cxxprogram_ps3):
	"Links object files into c++ shared libraries"
	run_str = '${LINK_CXX} ${LINKFLAGS} -zgenstub -zgenprx -mprx ${LIBPATH_ST:LIBPATH} ${LIB_ST:LIB} ${STLIBPATH_ST:STLIBPATH} ${STLIB_ST:STLIB} -Wl,--start-group ${CXXLNK_SRC_F}${SRC} ${LIB} -Wl,--end-group ${CXXLNK_TGT_F} ${TGT[0].abspath()}'
	inst_to = '${LIBDIR}'

@TaskGen.extension('.cpp','.cc','.cxx','.C','.c++')
def cxx_ps3_hook(self, node):
	return self.create_compiled_task('cxx_ps3', node)

if not '.c' in TaskGen.task_gen.mappings:
	TaskGen.task_gen.mappings['.c'] = TaskGen.task_gen.mappings['.cpp']

@TaskGen.extension('.c')
def c_hook(self, node):
	"Binds the c file extensions create :py:class:`waflib.Tools.c.c` instances"
	if not self.env.CC and self.env.CXX:
		return self.create_compiled_task('cxx_ps3', node)
	return self.create_compiled_task('c_ps3', node)


@TaskGen.feature('c_ps3', 'cxx_ps3')
@TaskGen.after_method('propagate_uselib_vars', 'process_source')
def apply_incpaths(self):
	"""
	Task generator method that processes the attribute *includes*::

		tg = bld(features='includes', includes='.')

	The folders only need to be relative to the current directory, the equivalent build directory is
	added automatically (for headers created in the build directory). This enables using a build directory
	or not (``top == out``).

	This method will add a list of nodes read by :py:func:`waflib.Tools.ccroot.to_incnodes` in ``tg.env.INCPATHS``,
	and the list of include paths in ``tg.env.INCLUDES``.
	"""

	lst = self.to_incnodes(self.to_list(getattr(self, 'includes', [])) + self.env.INCLUDES)
	self.includes_nodes = lst
	cwd = self.get_cwd()
	self.env.INCPATHS = [x.path_from(cwd) for x in lst]


@TaskGen.feature('c_ps3', 'cxx_ps3')
@TaskGen.after_method('process_source')
def apply_link(self):
	"""
	Collect the tasks stored in ``compiled_tasks`` (created by :py:func:`waflib.Tools.ccroot.create_compiled_task`), and
	use the outputs for a new instance of :py:class:`waflib.Tools.ccroot.link_task`. The class to use is the first link task
	matching a name from the attribute *features*, for example::

			def build(bld):
				tg = bld(features='cxx cxxprogram cprogram', source='main.c', target='app')

	will create the task ``tg.link_task`` as a new instance of :py:class:`waflib.Tools.cxx.cxxprogram`
	"""

	for x in self.features:
		if x == 'cprogram_ps3' and 'cxx_ps3' in self.features: # limited compat
			x = 'cxxprogram_ps3'
		elif x == 'cshlib_ps3' and 'cxx_ps3' in self.features:
			x = 'cxxshlib_ps3'

		if x in Task.classes:
			if issubclass(Task.classes[x], link_task):
				link = x
				break
	else:
		return

	objs = [t.outputs[0] for t in getattr(self, 'compiled_tasks', [])]
	self.link_task = self.create_task(link, objs)
	self.link_task.add_target(self.target)

	# remember that the install paths are given by the task generators
	try:
		inst_to = self.install_path
	except AttributeError:
		inst_to = self.link_task.inst_to
	if inst_to:
		# install a copy of the node list we have at this moment (implib not added)
		self.install_task = self.add_install_files(
			install_to=inst_to, install_from=self.link_task.outputs[:],
			chmod=self.link_task.chmod, task=self.link_task)

"""
@TaskGen.feature('cxxprogram_ps3')
@TaskGen.after_method('apply_link')
def apply_fself(self):
	elffile = self.link_task.outputs[0]
	in_nodes = [elffile]

	fselffile = elffile.change_ext('.prx')
	out_nodes = [fselffile]

	self.env.FSELFFILE = str(fselffile)

	self.fself_task = self.create_task('mkfself', in_nodes)
	self.fself_task.set_outputs(out_nodes)
	
	
	inst_to = getattr(self, 'ps3_install_path', None)
	if inst_to:
		self.add_install_files(install_to=inst_to,
			install_from=self.fself_task.outputs[:], chmod=Utils.O755, task=self.fself_task)
"""