#! /usr/bin/env python
# encoding: utf-8

# add PCH support to 'cxx'
from waflib.TaskGen import feature, after
from waflib.Task import Task
from waflib.Tools import c_preproc

@feature('cxx')
@after('apply_link')
def process_pch(self):
	if getattr(self, 'pch', ''):
		nodes = self.to_nodes(self.pch)
		for x in nodes:
			self.create_task('gchx', x, x.change_ext('.h.gch'))

class gchx(Task):
	run_str = '${CXX} ${CXXFLAGS} ${FRAMEWORKPATH_ST:FRAMEWORKPATH} ${CPPPATH_ST:INCPATHS} ${DEFINES_ST:DEFINES} ${CXX_SRC_F}${SRC} ${CXX_TGT_F}${TGT}'
	scan    = c_preproc.scan
	ext_out = ['.h']
	color   = 'BLUE'
