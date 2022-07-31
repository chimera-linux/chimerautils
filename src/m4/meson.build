m4_prog = executable(
    'm4',
    [
        'eval.c', 'expr.c', 'look.c', 'main.c', 'misc.c', 'gnum4.c', 'trace.c',
        yacc.process('parser.y'), lex.process('tokenizer.l'),
    ],
    include_directories : inc,
    link_with : [ libcompat ],
    dependencies : [ libm, ],
    c_args : [ '-DEXTENDED' ],
    install : true,
)

install_man('m4.1')
