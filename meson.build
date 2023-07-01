project(
    'miniisa',
    'c',
    version: '0.1.0',
    meson_version: '>=0.63.0',
    default_options: [
        'c_std=c11',
        # 'werror=true',
    ]
)
my_version = meson.project_version()

do_tests = get_option('do_tests')

my_inc = include_directories('include')

subdir('include/miniisa')
subdir('src/miniisa')
subdir('src/minias')

# don't build tests if subproject
if (not meson.is_subproject()) and do_tests
    catch2 = subproject('catch2')
    catch2_dep = catch2.get_variable('catch2_dep')
    subdir('tests/minias')
endif
