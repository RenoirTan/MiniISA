# config.h
test_conf_data = configuration_data()

proj_root = meson.project_source_root()
test_conf_data.set_quoted('MINITEST_PROJ_ROOT', proj_root)

test_dir = proj_root + '/tests'
test_conf_data.set_quoted('MINITEST_TEST_DIR', test_dir)

fodder_dir = test_dir + '/fodder'
test_conf_data.set_quoted('MINITEST_FODDER_DIR', fodder_dir)

configure_file(input: 'config.h.in', output: 'config.h', configuration: test_conf_data)
