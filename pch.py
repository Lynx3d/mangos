#! /usr/bin/env python
# encoding: utf-8

# add PCH support to 'cxx'
from waflib.TaskGen import feature, after
from waflib.Task import Task
from waflib.Tools import c_preproc
from waflib.Configure import conf

@feature('cxx')
@after('process_use')
def propagate_pch_feature_vars(self):
	"""
	each compiler defines variables like 'CXXFLAGS_cxxshlib', 'CXXFLAGS_cxxprogram', etc
	so when we make a task generator of the type cshlib, CXXFLAGS are modified accordingly
	we need to modify CXXFLAGS_PCH in same manner:
	"""
	env = self.env

	for x in self.features:
		compvar = 'CXXFLAGS_%s' % (x) # var = CXXFLAGS_PCH, x = cxxshlib => CXXFLAGS_PCH_cxxshlib
		env.append_value('CXXFLAGS_PCH', env[compvar])

@feature('cxx')
@after('apply_link')
def process_pch(self):
	if getattr(self, 'pch', ''):
		nodes = self.to_nodes(self.pch)
		for x in nodes:
			self.create_task('gchx', x, x.change_ext('.h.gch'))

class gchx(Task):
	run_str = '${CXX} ${CXXFLAGS_PCH} ${FRAMEWORKPATH_ST:FRAMEWORKPATH} ${CPPPATH_ST:INCPATHS} ${DEFINES_ST:DEFINES} ${CXX_SRC_F}${SRC} ${CXX_TGT_F}${TGT}'
	scan    = c_preproc.scan
	ext_out = ['.h']
	color   = 'BLUE'
