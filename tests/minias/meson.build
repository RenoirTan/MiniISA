test_minias_token_src = ['token.c']
test_minias_token_exe = executable(
    'test_minias_token',
    test_minias_token_src,
    include_directories: my_inc,
    link_with: minias_lib
)
test('token', test_minias_token_exe, suite: 'minias')
