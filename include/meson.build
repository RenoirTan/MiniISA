conf_data = configuration_data()

conf_data.set_quoted('MINIISA_VERSION', '0.1.0')
buildtype = get_option('buildtype')

is_debug = buildtype == 'debug'
conf_data.set('MINIISA_DEBUG', is_debug)

configure_file(input: 'config.h.in', output: 'config.h', configuration: conf_data)
