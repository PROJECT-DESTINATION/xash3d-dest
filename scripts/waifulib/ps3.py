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
import os

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
	run_str = '${LINK_SNC} ${LINKFLAGS} -oformat=fself ${LIBPATH_ST:LIBPATH} ${LIB_ST:LIB} ${LIBPATH_ST:STLIBPATH} ${LIB_ST:STLIB} -Wl,--start-group ${CCLNK_SRC_F}${SRC} ${LIB} -Wl,--end-group ${SNCLNK_TGT_F} ${TGT[0].abspath()}'

class cxxprogram_ps3(cxx.cxxprogram):
	run_str = '${LINK_SNC} ${LINKFLAGS} -oformat=fself ${LIBPATH_ST:LIBPATH} ${LIB_ST:LIB} ${LIBPATH_ST:STLIBPATH} ${LIB_ST:STLIB} --start-group ${CXXLNK_SRC_F}${SRC} ${LIB} --end-group ${SNCLNK_TGT_F} ${TGT[0].abspath()}'

class cxx_ps3(cxx.cxx):
	run_str = '${SNC} ${ARCH_ST:ARCH} ${CXXFLAGS} ${FRAMEWORKPATH_ST:FRAMEWORKPATH} -I"." ${SNCPATH_ST:INCPATHS} ${DEFINES_ST:DEFINES} ${SNC_SRC_F}${SRC} ${SNC_TGT_F}${TGT[0].abspath()} ${CPPFLAGS}'

class c_ps3(c.c):
	run_str = '${SNC} ${ARCH_ST:ARCH} ${CFLAGS} ${FRAMEWORKPATH_ST:FRAMEWORKPATH} -I"." ${SNCPATH_ST:INCPATHS} ${DEFINES_ST:DEFINES} ${SNC_SRC_F}${SRC} ${SNC_TGT_F}${TGT[0].abspath()} ${CPPFLAGS}'
	

class cxxshlib_ps3(cxxprogram_ps3):
	"Links object files into c++ shared libraries"
	run_str = '${LINK_SNC} ${LINKFLAGS} -oformat=fsprx ${LIBPATH_ST:LIBPATH} ${LIB_ST:LIB} ${LIBPATH_ST:STLIBPATH} ${LIB_ST:STLIB} --start-group ${CXXLNK_SRC_F}${SRC} ${LIB} --end-group ${SNCLNK_TGT_F} ${TGT[0].abspath()}'
	inst_to = '${LIBDIR}'

class cstlib_ps3(c.cstlib):
	"Links object files into c++ shared libraries"
	run_str = '${ARL} -M ${TGT[0].abspath()}.snarl | ${TGT[0].abspath()} ${STLIB_ST:SRC} save end'
	inst_to = '${LIBDIR}'
	def exec_command(self, *k, **kw):
		
		with open(k[0][2],"w") as f:
			f.write("create " + "\n".join(k[0][4:]))
		k = [k[0][:3]]
		ret = super().exec_command(*k, **kw)
		return ret

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
def apply_incpaths_ps3(self):
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
@TaskGen.before_method('apply_incpaths_ps3', 'propagate_uselib_vars')
@TaskGen.after_method('apply_link_ps3', 'process_source')
def process_use_ps3(self):
	"""
	Process the ``use`` attribute which contains a list of task generator names::

		def build(bld):
			bld.shlib(source='a.c', target='lib1')
			bld.program(source='main.c', target='app', use='lib1')

	See :py:func:`waflib.Tools.ccroot.use_rec`.
	"""

	use_not = self.tmp_use_not = set()
	self.tmp_use_seen = [] # we would like an ordered set
	use_prec = self.tmp_use_prec = {}
	self.uselib = self.to_list(getattr(self, 'uselib', []))
	self.includes = self.to_list(getattr(self, 'includes', []))
	names = self.to_list(getattr(self, 'use', []))

	for x in names:
		self.use_rec(x)

	for x in use_not:
		if x in use_prec:
			del use_prec[x]

	# topological sort
	out = self.tmp_use_sorted = []
	tmp = []
	for x in self.tmp_use_seen:
		for k in use_prec.values():
			if x in k:
				break
		else:
			tmp.append(x)

	while tmp:
		e = tmp.pop()
		out.append(e)
		try:
			nlst = use_prec[e]
		except KeyError:
			pass
		else:
			del use_prec[e]
			for x in nlst:
				for y in use_prec:
					if x in use_prec[y]:
						break
				else:
					tmp.append(x)
	if use_prec:
		raise Errors.WafError('Cycle detected in the use processing %r' % use_prec)
	out.reverse()
	
	link_task = getattr(self, 'link_task', None)
	for x in out:
		y = self.bld.get_tgen_by_name(x)
		var = y.tmp_use_var
		if var and link_task:
			if self.env.SKIP_STLIB_LINK_DEPS and isinstance(link_task, stlink_task):
				# If the skip_stlib_link_deps feature is enabled then we should
				# avoid adding lib deps to the stlink_task instance.
				pass
			elif var == 'LIB' or y.tmp_use_stlib or x in names:
				self.env.append_value(var, [y.target[y.target.rfind(os.sep) + 1:]])
				self.link_task.dep_nodes.extend(y.link_task.outputs)
				tmp_path = y.link_task.outputs[0].parent.path_from(self.get_cwd())
				self.env.append_unique(var + 'PATH', [tmp_path])
		else:
			if y.tmp_use_objects:
				self.add_objects_from_tgen(y)

		if getattr(y, 'export_includes', None):
			print(self,y.to_incnodes(y.export_includes))
			# self.includes may come from a global variable #2035
			self.includes = self.includes + y.to_incnodes(y.export_includes)

		if getattr(y, 'export_defines', None):
			self.env.append_value('DEFINES', self.to_list(y.export_defines))

	# and finally, add the use variables (no recursion needed)
	for x in names:
		try:
			y = self.bld.get_tgen_by_name(x)
		except Errors.WafError:
			if not self.env['STLIB_' + x] and not x in self.uselib:
				self.uselib.append(x)
		else:
			for k in self.to_list(getattr(y, 'use', [])):
				if not self.env['STLIB_' + k] and not k in self.uselib:
					self.uselib.append(k)


@TaskGen.feature('c_ps3', 'cxx_ps3')
@TaskGen.after_method('process_source')
def apply_link_ps3(self):
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