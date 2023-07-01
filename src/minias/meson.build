minias_lib_src = ['tokenizer.c', 'token.c', 'parser.c', 'prebytecode.c']
minias_lib = library(
    'minias',
    minias_lib_src,
    include_directories: my_inc,
    install: true,
    link_with: miniisa_lib
)

minias_exe_src = ['main.c']
minias_exe = executable(
    'minias',
    minias_exe_src,
    include_directories: my_inc,
    install: true,
    link_with: minias_lib
)
