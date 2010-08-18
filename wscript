#! /usr/bin/env python
# encoding: utf-8

# the following two variables are used by the target "waf dist"
VERSION='0.17'
APPNAME='MaNGOS'

top = '.'
out = 'build'

def options(opt):
	opt.tool_options('compiler_c')
	opt.tool_options('compiler_cxx')
	opt.add_option('-d', '--debug', dest='debug', default=True, help='enable debug info (-g)')

def configure(conf):
	conf.check_tool('compiler_c')
	conf.check_tool('compiler_cxx')
	conf.env.append_value('CXXFLAGS', ['-O2'])
	conf.env.append_value('INCLUDES', '.')
	if conf.options.debug:
		conf.env.append_unique('CXXFLAGS', ['-g'])
	conf.env.append_value('CONTRIB_DEFINES', ['MMAP_GENERATOR','NO_CORE_FUNCS'])
	conf.check_cc(lib = 'z', uselib_store='ZLIB', mandatory = True)
	# TODO: detect ACE
	conf.env.append_value('INCLUDES', '../../../dep/ACE_wrappers')
	conf.check_cfg(path='mysql_config', msg='Checking for mysql', package='', uselib_store='MYSQL', args='--include --libs_r')
	# mmaps
	conf.check_cfg(atleast_pkgconfig_version='0.0.0')
	try:
		conf.check_cfg(package='sdl', args='--libs')
		conf.env['HAVE_SDL'] = True
	except:
		conf.env['HAVE_SDL'] = False

def build(bld):
	# build genrevision tool first (used by src/shared/wscript_build)
	bld.program(source = 'src/tools/genrevision/genrevision.cpp', target = 'genrevision')
	bld.add_group()

	# generate revision.h
	#tg = bld.get_tgen_by_name('genrevision')
	bld(rule = '${bld.bldnode.abspath()}/genrevision ${bld.srcnode.abspath()}', target = 'revision.h')
	bld.recurse('src/shared')
	bld.recurse('src/framework')
	bld.recurse('src/game')
	bld.recurse('dep/src/g3dlite')
	# mmaps
	# bld.recurse('src/shared/pathfinding')
	# bld.recurse('contrib/mmap')

