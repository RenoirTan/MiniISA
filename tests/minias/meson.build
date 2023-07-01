test_minias_token_src = ['test_token.cpp']
test_minias_token_exe = executable(
    'test_minias_token',
    test_minias_token_src,
    include_directories: my_inc,
    link_with: minias_lib,
    dependencies: [catch2_dep]
)
test('token', test_minias_token_exe, suite: 'minias')
