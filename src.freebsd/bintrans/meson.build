bintrans_prog = executable(
    'bintrans',
    ['bintrans.c', 'uuencode.c', 'uudecode.c', 'qp.c'],
    include_directories: [inc, cinc],
    link_with: [libcompat],
    install: true,
    install_dir: bindir,
)

install_man('bintrans.1')
install_man('uuencode.format.5')

foreach x: ['uuencode', 'uudecode', 'b64encode', 'b64decode', 'base64']
    install_symlink(x, pointing_to: 'bintrans', install_dir: bindir)
    install_symlink(x + '.1', pointing_to: 'bintrans.1', install_dir: man1dir)
endforeach

install_symlink(
    'uuencode.5', pointing_to: 'uuencode.format.5', install_dir: man5dir
)
