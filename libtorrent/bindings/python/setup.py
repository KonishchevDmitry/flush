#!/usr/bin/env python

from distutils import sysconfig
from distutils.core import setup, Extension
import os
import platform
import sys

if '-lboost_python-mt' == '':
	print 'You need to pass --enable-python-binding to configure in order ',
	print 'to properly use this setup. There is no boost.python library configured now'
	sys.exit(1)

def parse_cmd(cmdline, prefix, keep_prefix = False):
	ret = []
	for token in cmdline.split():
		if token[:len(prefix)] == prefix:
			if keep_prefix:
				ret.append(token)
			else:
				ret.append(token[len(prefix):])
	return ret

def arch():
	if platform.system() != 'Darwin': return []
	a = os.uname()[4]
	if a == 'Power Macintosh': a = 'ppc'
	return ['-arch', a]

if platform.system() == 'Windows':
# on windows, build using bjam and build an installer
	import shutil
	# msvc 9.0 (2008) is the official windows compiler for python 2.6
	# http://docs.python.org/whatsnew/2.6.html#build-and-c-api-changes
	if os.system('bjam boost=source link=static geoip=static boost-link=static release msvc-9.0 optimization=space') != 0:
		print 'build failed'
		sys.exit(1)
	try: os.mkdir(r'build')
	except: pass
	try: os.mkdir(r'build\lib')
	except: pass
	try: os.mkdir(r'libtorrent')
	except: pass
	shutil.copyfile(r'bin\msvc-9.0\release\boost-source\geoip-static\link-static\optimization-space\threading-multi\libtorrent.pyd', r'.\build\lib\libtorrent.pyd')
	setup( name='python-libtorrent',
		version='0.15.4',
		author = 'Arvid Norberg',
		author_email='arvid@cs.umu.se',
		description = 'Python bindings for libtorrent-rasterbar',
		long_description = 'Python bindings for libtorrent-rasterbar',
		url = 'http://www.rasterbar.com/products/libtorrent/index.html',
		platforms = 'Windows',
		license = 'Boost Software License - Version 1.0 - August 17th, 2003',
		packages = ['libtorrent'],
	)
	sys.exit(0)

config_vars = sysconfig.get_config_vars()
if "CFLAGS" in config_vars and "-Wstrict-prototypes" in config_vars["CFLAGS"]:
	config_vars["CFLAGS"] = config_vars["CFLAGS"].replace("-Wstrict-prototypes", " ")
if "OPT" in config_vars and "-Wstrict-prototypes" in config_vars["OPT"]:
	config_vars["OPT"] = config_vars["OPT"].replace("-Wstrict-prototypes", " ")

source_list = os.listdir(os.path.join(os.path.dirname(__file__), "src"))
source_list = [os.path.join("src", s) for s in source_list if s.endswith(".cpp")]

extra_cmd = '-DTORRENT_USE_OPENSSL -DWITH_SHIPPED_GEOIP_H -DBOOST_ASIO_HASH_MAP_BUCKETS=1021 -DBOOST_EXCEPTION_DISABLE -DTORRENT_LINKING_SHARED  -I/opt/local/include    -lboost_system-mt -lboost_filesystem-mt -lboost_thread-mt -lboost_python-mt  -lssl -lcrypto -lz   -L/opt/local/lib   -I/opt/local/include  '

setup( name='python-libtorrent',
	version='0.15.4',
	author = 'Arvid Norberg',
	author_email='arvid@cs.umu.se',
	description = 'Python bindings for libtorrent-rasterbar',
	long_description = 'Python bindings for libtorrent-rasterbar',
	url = 'http://www.rasterbar.com/products/libtorrent/index.html',
	platforms = 'any',
	license = 'Boost Software License - Version 1.0 - August 17th, 2003',
	ext_modules = [Extension('libtorrent',
		sources = source_list,
		language='c++',
		include_dirs = ['../../include'] + parse_cmd(extra_cmd, '-I'),
		library_dirs = ['../../src/.libs'] + parse_cmd(extra_cmd, '-L'),
		extra_link_args = '-L/opt/local/lib '.split() + arch(),
		extra_compile_args = parse_cmd(extra_cmd, '-D', True) + arch(),
		libraries = ['torrent-rasterbar'] + parse_cmd(extra_cmd, '-l'))],
)
