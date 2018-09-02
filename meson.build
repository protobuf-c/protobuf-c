project('protobuf-c',
  ['c', 'cpp'],
  version : '1.3.1',
  default_options : [
    'buildtype=debugoptimized',
    'c_std=c99',
    'cpp_std=c++14',
  ],
  meson_version : '>= 0.47.0',
)

cc = meson.get_compiler('c')
cpp = meson.get_compiler('cpp')
pkgconfig = import('pkgconfig')

possible_cc_flags = '''
-Wchar-subscripts
-Wdeclaration-after-statement
-Werror=incompatible-pointer-types
-Werror=int-conversion
-Wformat-security
-Wmissing-declarations
-Wmissing-prototypes
-Wnested-externs
-Wnull-dereference
-Wpointer-arith
-Wshadow
-Wsign-compare
-Wstrict-prototypes
-Wtype-limits
'''.split()
add_project_arguments(cc.get_supported_arguments(possible_cc_flags), language : 'c')

#
##
### dependencies
##
#

if get_option('build-compiler')
  protobuf = dependency('protobuf',
    version : '>= 2.6.0',
    required : true,
    native : true)

  libprotoc = cpp.find_library('protoc',
    required : true,
    dirs : protobuf.get_pkgconfig_variable('libdir'))
endif

#
##
### config.h
##
#

conf = configuration_data()

conf.set_quoted('PACKAGE_STRING',   meson.project_name() + ' ' + meson.project_version())
conf.set_quoted('PACKAGE_VERSION',  meson.project_version())

conf.set('WORDS_BIGENDIAN', host_machine.endian() == 'big')
conf.set('HAVE_PROTO3', protobuf.version().version_compare('>= 3.0.0'))

configure_file(output : 'config.h', configuration : conf)

add_project_arguments('-include', 'config.h', language : ['c', 'cpp'])

#
##
### libprotobuf-c
##
#

libprotobuf_c_sym = 'protobuf-c/libprotobuf-c.sym'
libprotobuf_c_sym_path = join_paths(meson.current_source_dir(), libprotobuf_c_sym)

libprotobuf_c = both_libraries('protobuf-c',
  'protobuf-c/protobuf-c.c',
  link_args : ['-Wl,--version-script=' + libprotobuf_c_sym_path],
  link_depends : libprotobuf_c_sym,
  soversion : '1',
  version : '1.0.0',
  install : true,
)

pkgconfig.generate(
  name : 'libprotobuf-c',
  description : 'Protocol Buffers C library',
  version : meson.project_version(),
  libraries : libprotobuf_c.get_shared_lib(),
)

install_headers('protobuf-c/protobuf-c.h', subdir : 'protobuf-c')

# Support the older include path <google/protobuf-c/protobuf-c.h>
if build_machine.system() != 'windows'
  google_includedir = join_paths('${MESON_INSTALL_DESTDIR_PREFIX}', get_option('includedir'), 'google', 'protobuf-c')
  google_protobuf_c_header = join_paths(google_includedir, 'protobuf-c.h')
  meson.add_install_script('sh', '-x', '-c', 'mkdir -p ' + google_includedir)
  meson.add_install_script('sh', '-x', '-c', 'ln -sf ../../protobuf-c/protobuf-c.h ' + google_protobuf_c_header)
endif

#
##
### protoc-c
##
#

if get_option('build-compiler')
  protoc_gen_c_sources = files('''
    protoc-c/c_bytes_field.cc
    protoc-c/c_bytes_field.h
    protoc-c/c_enum.cc
    protoc-c/c_enum.h
    protoc-c/c_enum_field.cc
    protoc-c/c_enum_field.h
    protoc-c/c_extension.cc
    protoc-c/c_extension.h
    protoc-c/c_field.cc
    protoc-c/c_field.h
    protoc-c/c_file.cc
    protoc-c/c_file.h
    protoc-c/c_generator.cc
    protoc-c/c_generator.h
    protoc-c/c_helpers.cc
    protoc-c/c_helpers.h
    protoc-c/c_message.cc
    protoc-c/c_message.h
    protoc-c/c_message_field.cc
    protoc-c/c_message_field.h
    protoc-c/c_primitive_field.cc
    protoc-c/c_primitive_field.h
    protoc-c/c_service.cc
    protoc-c/c_service.h
    protoc-c/c_string_field.cc
    protoc-c/c_string_field.h
    protoc-c/main.cc
  '''.split())

  protoc_c = executable('protoc-c',
    protoc_gen_c_sources,
    dependencies : [protobuf, libprotoc],
    install : true,
    native : true,
  )

  # Create protoc-gen-c as a symlink to protoc-c
  if build_machine.system() != 'windows'
    install_protoc_gen_c = join_paths('${MESON_INSTALL_DESTDIR_PREFIX}', get_option('bindir'), 'protoc-gen-c')
    meson.add_install_script('sh', '-x', '-c', 'ln -sf protoc-c ' + install_protoc_gen_c)
  endif
endif

#
##
### tests
##
#

if get_option('build-compiler') and get_option('run-tests')
  abs_protoc_path = join_paths(protobuf.get_pkgconfig_variable('exec_prefix'), 'bin', 'protoc')

  protoc = find_program([abs_protoc_path, 'protoc'],
    required : true,
    native : true)

  gen_protobuf = generator(protoc,
    output    : ['@BASENAME@.pb.cc', '@BASENAME@.pb.h'],
    arguments : ['--proto_path=@CURRENT_SOURCE_DIR@', '--cpp_out=@BUILD_DIR@', '@INPUT@'])

  gen_protobuf_c = generator(protoc_c,
    output    : ['@BASENAME@.pb-c.c', '@BASENAME@.pb-c.h'],
    arguments : ['--proto_path=@CURRENT_SOURCE_DIR@', '--c_out=@BUILD_DIR@', '@INPUT@'])

  test('version', executable('t_version', 't/version/version.c', link_with : libprotobuf_c))

  test('issue220',
    executable('t_issue220', 't/issue220/issue220.c',
      gen_protobuf_c.process('t/issue220/issue220.proto',
        preserve_path_from : meson.current_source_dir()),
      link_with : libprotobuf_c))

  test('issue251',
    executable('t_issue251', 't/issue251/issue251.c',
      gen_protobuf_c.process('t/issue251/issue251.proto',
        preserve_path_from : meson.current_source_dir()),
      link_with : libprotobuf_c))

  test('generated-code',
    executable('t_test-generated-code', 't/generated-code/test-generated-code.c',
      gen_protobuf_c.process('t/test.proto',
        preserve_path_from : meson.current_source_dir()),
      link_with : libprotobuf_c))

  gen_cxx_data = custom_target('cxx-generate-packed-data',
    output : ['test-full-cxx-output.inc'],
    command :
      executable('cxx-generate-packed-data', 't/generated-code2/cxx-generate-packed-data.cc',
        gen_protobuf.process('t/test-full.proto',
          preserve_path_from : meson.current_source_dir()),
        dependencies : protobuf),
    capture : true)

  test('generated-code2',
    executable('t_test-generated-code2', 't/generated-code2/test-generated-code2.c', gen_cxx_data,
      gen_protobuf_c.process('t/test-full.proto', 't/test-optimized.proto',
        preserve_path_from : meson.current_source_dir()),
      link_with : libprotobuf_c))

  if protobuf.version().version_compare('>= 3.0.0')
    test('generated-code3',
      executable('t_test-generated-code3', 't/generated-code/test-generated-code.c',
        gen_protobuf_c.process('t/test-proto3.proto',
          preserve_path_from : meson.current_source_dir()),
        c_args : ['-DPROTO3'],
        link_with : libprotobuf_c))
  endif
endif

#
##
### doxygen documentation
##
#

if get_option('build-docs')
  doxygen = find_program('doxygen', required : false)
  if doxygen.found()
    cdoxy = configuration_data()
    cdoxy.set('PACKAGE',              'protobuf-c')
    cdoxy.set('PACKAGE_DESCRIPTION',  'Protocol Buffers implementation in C')
    cdoxy.set('PACKAGE_VERSION',      meson.project_version())
    cdoxy.set('top_srcdir',           meson.source_root())
    cdoxy.set('DOXYGEN_INPUT',        join_paths(meson.source_root(), 'protobuf-c'))

    doxyfile = configure_file(
      input : 'Doxyfile.in',
      output : 'Doxyfile',
      configuration : cdoxy,
      install : false)

    doxygen_datadir = join_paths(get_option('datadir'), 'doc', 'protobuf-c')

    html_target = custom_target('html',
      input : doxyfile,
      output : 'html',
      command : [doxygen, doxyfile],
      install : true,
      install_dir : doxygen_datadir)
  endif
endif

#
##
### valgrind test setup
##
#

valgrind = find_program('valgrind', required : false)
if valgrind.found()
  add_test_setup('valgrind', exe_wrapper : [valgrind, '--quiet', '--error-exitcode=1', '--leak-check=full'])
endif
