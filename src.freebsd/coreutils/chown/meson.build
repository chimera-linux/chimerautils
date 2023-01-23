chown_prog = executable(
    'chown',
    [ 'chown.c' ],
    c_args : [ '-DUID_MAX=INT_MAX', '-DGID_MAX=INT_MAX' ],
    include_directories : inc,
    dependencies : [ libfts, ],
    install : true,
    install_dir : sbindir,
)

install_symlink('chgrp', pointing_to: 'chown', install_dir: sbindir)

install_man('chown.8')
install_man('chgrp.1')
