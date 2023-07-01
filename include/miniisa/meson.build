# config.h
conf_data = configuration_data()

conf_data.set_quoted('MINIISA_VERSION', my_version)
buildtype = get_option('buildtype')

is_debug = buildtype == 'debug'
conf_data.set('MINIISA_DEBUG', is_debug)

configure_file(input: 'config.h.in', output: 'config.h', configuration: conf_data)

# Install headers
my_headers = ['bytecode.h', 'instruction.h', 'permissions.h', 'section.h', 'symbol.h']
install_headers(my_headers, preserve_path: true, subdir: 'miniisa')
build_dir = meson.current_build_dir() + '/'
_built_headers = ['config.h']
built_headers = []
foreach p : _built_headers
    built_headers += [build_dir + p]
endforeach
install_headers(built_headers, subdir: 'miniisa')
