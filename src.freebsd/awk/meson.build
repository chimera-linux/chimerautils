if byacc.found()
    awkgram_cmd = [byacc, '-d', '-H', '@OUTPUT1@', '-o', '@OUTPUT0@', '@INPUT@']
else
    awkgram_cmd = [bison, '--defines=@OUTPUT1@', '-o', '@OUTPUT0@', '@INPUT@']
endif

awkgram_ch = custom_target(
    'awkgram_ch',
    input: ['awkgram.y'],
    output: ['awkgram.tab.c', 'awkgram.tab.h'],
    command: awkgram_cmd,
)

maketab = executable(
    'maketab', ['maketab.c', awkgram_ch[1]],
    c_args: ['-D_GNU_SOURCE'],
    native: true
)

proctab_src = custom_target(
    'awk_proctab_src',
    input: [awkgram_ch[1]],
    output: ['proctab.c'],
    command: [maketab, '@INPUT@'],
    capture: true
)

extra_cargs = []

if cc.has_argument('-Wunused-parameter')
    extra_cargs += ['-Wno-unused-parameter']
endif

awk_prog = executable(
    'awk',
    [
        'b.c', 'lex.c', 'lib.c', 'main.c', 'parse.c', 'run.c', 'tran.c',
        awkgram_ch, proctab_src
    ],
    include_directories: [inc],
    dependencies: [libm],
    c_args: extra_cargs,
    install: true,
)

install_man('awk.1')
