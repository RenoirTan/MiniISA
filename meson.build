project(
    'miniisa',
    'c',
    version: '0.1.0',
    meson_version: '>=0.53.0',
    default_options: [
        'c_std=c11',
        # 'werror=true',
    ]
)
my_version = meson.project_version()

my_inc = include_directories('include')

subdir('include/miniisa')
subdir('src/miniisa')
subdir('src/minias')
