#!/usr/bin/python3

import argparse
import os
from packaging.version import Version
from pathlib import Path
import re
import shutil

ARCH_DEFAULT_VERSION_MAP = {
	'i486': '3.9999',
	'i686': '4.0',
	'x86_64': '5.2',
	'aarch64': '10.0.19041',
}

ARCH_LIB_DIR_MAP = {
	'i486': 'lib32',
	'i686': 'lib32',
	'x86_64': 'lib64',
	'aarch64': 'libarm64',
}

ARCH_CPPFLAGS_MAP = {
	'i486': 'CPPFLAGS32',
	'i686': 'CPPFLAGS32',
	'x86_64': 'CPPFLAGS64',
	'aarch64': 'CPPFLAGSARM64',
}

ARCH_DT_DEF_COMMAND_MAP = {
	'i486': 'DTDEF32',
	'i686': 'DTDEF32',
	'x86_64': 'DTDEF64',
	'aarch64': 'DTDEFARM64',
}

def parse_args():
	parser = argparse.ArgumentParser(
		description = 'Patch mingw-w64 source code'
	)
	parser.add_argument(
		'source',
		type = Path,
		help = 'Path to mingw-w64 source code'
	)
	parser.add_argument(
		'-a', '--arch',
		type = str,
		choices = ARCH_DEFAULT_VERSION_MAP.keys(),
		required = True,
		help = 'Target architecture'
	)
	parser.add_argument(
		'--nt-ver',
		type = Version,
		help = 'Target Windows NT version'
	)
	args = parser.parse_args()
	if not args.nt_ver:
		args.nt_ver = Version(ARCH_DEFAULT_VERSION_MAP[args.arch])
	return args

def process_modules(args):
	modules = {}
	thunk_dir = args.source / 'mingw-w64-crt'/ 'thunk'
	thunk_dir.mkdir(parents = True, exist_ok = True)

	for inc_entry in os.scandir('include'):
		shutil.copy(inc_entry.path, thunk_dir)

	for entry in os.scandir('src-extra'):
		v = Version(entry.name)
		if v <= args.nt_ver:
			continue
		for mod_entry in os.scandir(entry.path):
			mod_name = mod_entry.name
			if mod_name not in modules:
				modules[mod_name] = []
			for fn_entry in os.scandir(mod_entry.path):
				shutil.copy(fn_entry.path, thunk_dir)
				modules[mod_name].append(fn_entry.name)
	return modules

def locate_mod_def_file(possible_paths):
	for path in possible_paths:
		if path.exists():
			return path
	raise FileNotFoundError('No suitable .def file found')

def patch_project(args, modules):
	# load automake template
	AM_ADDITION_START = '# <<< mingw-thunk addition\n'
	AM_ADDITION_END = '# >>> mingw-thunk addition\n'
	mf_file = args.source / 'mingw-w64-crt' / 'Makefile.am'
	with open(mf_file, 'r') as f:
		mf_content = f.readlines()
	addition_start = -1
	addition_end = -1
	for i, line in enumerate(mf_content):
		if line == AM_ADDITION_START:
			addition_start = i
		if line == AM_ADDITION_END:
			addition_end = i
	if addition_start >= 0:
		mf_content = mf_content[:addition_start] + mf_content[addition_end+1:]

	# patch makefile cflags
	cflags_line = -1
	cxxflags_line = -1
	cflags_pattern = re.compile(r'^AM_CFLAGS\s*=')
	cxxflags_pattern = re.compile(r'^AM_CXXFLAGS\s*=')
	for i, line in enumerate(mf_content):
		if cflags_pattern.match(line):
			cflags_line = i
		if cxxflags_pattern.match(line):
			cxxflags_line = i
	if cflags_line < 0 or cxxflags_line < 0:
		raise RuntimeError('Cannot find cflags or cxxflags in Makefile.am')
	cflags = cflags_pattern.sub('', mf_content[cflags_line]).strip().split()
	cxxflags = ['-fno-exceptions']
	for flag in cflags:
		if flag.startswith('-std='):
			cxxflags.append('-std=gnu++17')
		else:
			cxxflags.append(flag)
	mf_content[cxxflags_line] = 'AM_CXXFLAGS=' + ' '.join(cxxflags) + '\n'

	# process modules
	mf_addition = []
	lib_dir = ARCH_LIB_DIR_MAP[args.arch]
	arch_def_dir = args.source / 'mingw-w64-crt' / lib_dir
	common_def_dir = args.source / 'mingw-w64-crt' / 'lib-common'
	for mod_name, mod_function_files in modules.items():
		# find module def file
		mod_def_file_paths = [
			arch_def_dir / (mod_name + '.def.in'),
			arch_def_dir / (mod_name + '.def'),
			common_def_dir / (mod_name + '.def.in'),
			common_def_dir / (mod_name + '.def'),
		]
		mod_def_file = locate_mod_def_file(mod_def_file_paths)

		# patch module def file
		with open(mod_def_file, 'r') as f:
			mod_def_content = f.readlines()
		for fn_file in mod_function_files:
			fn = Path(fn_file).stem
			if args.arch in ('i486', 'i686'):
				pattern = re.compile('^' + fn + r'@\d*$')
			else:
				pattern = re.compile('^' + fn + '$')
			for i, line in enumerate(mod_def_content):
				if pattern.match(line):
					mod_def_content[i] = '; ' + line
		with open(mod_def_file, 'w') as f:
			f.writelines(mod_def_content)

		# add thunk files to makefile rules
		mf_src = list(map(lambda file: 'thunk/' + file, mod_function_files))
		mf_src_line = -1
		mf_src_pattern = re.compile('^src_lib' + mod_name + r'\s*=')
		for i, line in enumerate(mf_content):
			if mf_src_pattern.match(line):
				mf_src_line = i
		if mf_src_line >= 0:
			mf_addition.append(f'src_lib{mod_name} += ' + ' '.join(mf_src) + '\n')
		else:
			cppflags = ARCH_CPPFLAGS_MAP[args.arch]
			dt_def_command = ARCH_DT_DEF_COMMAND_MAP[args.arch]
			mf_addition.append(f'src_lib{mod_name} = ' + ' '.join(mf_src) + '\n')
			mf_addition.append(f'{lib_dir}_LIBRARIES += {lib_dir}/lib{mod_name}.a\n')
			mf_addition.append(f'{lib_dir}_lib{mod_name}_a_SOURCES = $(src_lib{mod_name})\n')
			mf_addition.append(f'{lib_dir}_lib{mod_name}_a_CPPFLAGS = $({cppflags}) $(sysincludes)\n')
			if mod_def_file.suffix == '.in':
				mf_addition.append(f'{lib_dir}_lib{mod_name}_a_AR = $({dt_def_command}) {lib_dir}/{mod_name}.def && $(AR) $(ARFLAGS)\n')
			else:
				rel_def_path = mod_def_file.relative_to(args.source / 'mingw-w64-crt')
				mf_addition.append(f'{lib_dir}_lib{mod_name}_a_AR = $({dt_def_command}) $(top_srcdir)/{rel_def_path} && $(AR) $(ARFLAGS)\n')

	with open(mf_file, 'w') as f:
		f.writelines(mf_content + [AM_ADDITION_START, f'if {lib_dir.upper()}\n'] + mf_addition + ['endif\n', AM_ADDITION_END])

def main():
	args = parse_args()
	modules = process_modules(args)
	patch_project(args, modules)

if __name__ == '__main__':
	main()
