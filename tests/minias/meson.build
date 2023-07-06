test_minias_token_src = ['test_token.cpp']
test_minias_token_exe = executable(
    'test_minias_token',
    test_minias_token_src,
    include_directories: my_inc,
    link_with: minias_lib,
    dependencies: [catch2_dep]
)
test('token', test_minias_token_exe, suite: 'minias')

test_minias_tokenizer_src = ['test_tokenizer.cpp']
test_minias_tokenizer_exe = executable(
    'test_minias_tokenizer',
    test_minias_tokenizer_src,
    include_directories: my_inc,
    link_with: minias_lib,
    dependencies: [catch2_dep]
)
test('tokenizer', test_minias_tokenizer_exe, suite: 'minias')
