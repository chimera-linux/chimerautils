patch_prog = executable(
    'patch',
    [
        'patch.c', 'backupfile.c', 'inp.c', 'mkpath.c', 'pch.c', 'util.c',
    ],
    include_directories : inc,
    link_with : [ libcompat ],
    install : true,
)

install_man('patch.1')
