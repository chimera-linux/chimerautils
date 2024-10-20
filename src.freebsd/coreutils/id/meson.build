id_deps = []

if libselinux.found()
    id_deps += [libselinux]
endif

id_prog = executable(
    'id',
    [ 'id.c' ],
    include_directories : inc,
    dependencies : id_deps,
    install : true,
)

install_symlink('groups', pointing_to: 'id', install_dir: bindir)
install_symlink('whoami', pointing_to: 'id', install_dir: bindir)

install_man('id.1')
install_man('groups.1')
install_man('whoami.1')
