#! /usr/bin/env python
# encoding: utf-8

import Utils

# the following two variables are used by the target "waf dist"
VERSION='0.17'
APPNAME='MaNGOS'

top = '.'
out = 'build'

def options(opt):
	opt.tool_options('compiler_c')
	opt.tool_options('compiler_cxx')
	opt.add_option('-d', '--debug', dest='debug', default=True, help='enable debug info (-g)')
	opt.add_option('--with-std-malloc', dest='std_malloc', default=False, action='store_true',
		help='use standard malloc instead of TBB')
	opt.add_option('--with-tools', dest='with_tools', default=False, action='store_true',
		help='build additional tools in contrib')
	opt.add_option('--with-sd2', dest='with_sd2', default=False, action='store_true',
		help='build ScriptDev2 in src/bindings/ScriptDev2; Sources must be available, obviously')

def configure(conf):
	conf.check_tool('compiler_c')
	conf.check_tool('compiler_cxx')
	conf.check_tool('pch', tooldir='.')

	conf.env.append_value('CXXFLAGS', ['-O2'])
	#conf.env.append_value('INCLUDES', '.')
	if conf.options.debug:
		conf.env.append_unique('CXXFLAGS', ['-g'])
	conf.env['WITH_TOOLS'] = conf.options.with_tools
	conf.env['WITH_SD2'] = conf.options.with_sd2
	conf.env.append_value('CONTRIB_DEFINES', ['MMAP_GENERATOR','NO_CORE_FUNCS'])
	conf.check_cc(lib = 'z', uselib_store='ZLIB', mandatory = True)
	conf.check_cc(lib = 'pthread', uselib_store='PTHREAD', mandatory = True)
	conf.check_cfg(atleast_pkgconfig_version='0.0.0')
	# detect SSL
	conf.check_cfg(package='libssl', args='--cflags --libs')
	# detect crypto (required by SSL...)
	conf.check_cfg(package='libcrypto', args='--cflags --libs')
	# detect ACE
	# TODO: build ACE from source if not available
	conf.check_cfg(package='ACE', args='--cflags --libs')
	conf.check(header_name='ace/Stack_Trace.h', compile_mode='cxx', define_name='HAVE_ACE_STACK_TRACE_H', mandatory=False)
	#conf.env.append_value('INCLUDES', '../../../dep/ACE_wrappers')
	# detect TBB (unless disabled)
	if conf.options.std_malloc:
		conf.env.append_value('DEFINES', 'USE_STANDARD_MALLOC')
	else:
		# TODO: build TBB from source if not available
		conf.check_cfg(package='tbb', args='--cflags --libs')
		conf.env.append_unique('LIB_TBB', ['tbbmalloc'])
	# detect MySQL with mysql_config
	conf.check_cfg(path='mysql_config', msg='Checking for mysql', package='', uselib_store='MYSQL', args='--include --libs_r')
	# *nix specific defines
	conf.env.append_value('DEFINES', ['HAVE_CONFIG_H', Utils.subst_vars('SYSCONFDIR="${PREFIX}/mangos/etc/"', conf.env)])

	# == tool setup ==
	if conf.env['WITH_TOOLS']:
		conf.check_cc(lib = 'bz2')
		conf.recurse('dep/libmpq')
		# mmaps, SDL for recast demo
		conf.check_cfg(package='sdl', args='--cflags --libs', mandatory=False)

	conf.write_config_header('config.h')
	# copy CXXFLAGS so we can add -include directive to CXXFLAGS for sources
	conf.env['CXXFLAGS_PCH'] = conf.env['CXXFLAGS']

def build(bld):
	# build genrevision tool first (used by src/shared/wscript_build)
	bld.program(source = 'src/tools/genrevision/genrevision.cpp', target = 'genrevision', install_path = None)
	bld.add_group()

	# generate revision.h
	#tg = bld.get_tgen_by_name('genrevision')
	bld(rule = '${bld.bldnode.abspath()}/genrevision -o ${TGT} ${bld.srcnode.abspath()}', target = 'src/shared/revision.h')
	if bld.env.WITH_SD2:
		bld.recurse('src/bindings/ScriptDev2')
	else:
		bld.recurse('src/bindings/universal')
	bld.recurse('src/shared')
	bld.recurse('src/framework')
	bld.recurse('src/game')
	bld.recurse('src/realmd')
	bld.recurse('src/mangosd')
	bld.recurse('dep/src/g3dlite')
	bld.recurse('dep/src/gsoap')
	if bld.env.WITH_TOOLS:
		bld.recurse('dep/libmpq/libmpq')
		bld.recurse('contrib/vmap_assembler')
		bld.recurse('contrib/vmap_extractor_v3')
	# mmaps
	# bld.recurse('src/shared/pathfinding')
	# bld.recurse('contrib/mmap')

	# install SQL updates
	start_dir = bld.path.find_dir('sql/updates')
	bld.install_files('${PREFIX}/mangos/dat/mangos/sql/updates', start_dir.ant_glob('*.sql'),
		relative_trick=False)

