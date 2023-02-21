setarch = executable('setarch', ['setarch.c'],
    include_directories: inc_main,
    install: true
)

install_symlink('linux32', pointing_to: 'setarch', install_dir: bindir)
install_symlink('linux64', pointing_to: 'setarch', install_dir: bindir)

fam = host_machine.cpu_family()

if fam == 'ia64' or fam == 'x86_64'
    install_symlink('i386', pointing_to: 'setarch', install_dir: bindir)
endif

if fam == 'mips64'
    install_symlink('mips', pointing_to: 'setarch', install_dir: bindir)
    install_symlink('mips32', pointing_to: 'setarch', install_dir: bindir)
endif

if fam == 'ppc64'
    install_symlink('ppc', pointing_to: 'setarch', install_dir: bindir)
    install_symlink('ppc32', pointing_to: 'setarch', install_dir: bindir)
endif

if fam == 's390x'
    install_symlink('s390', pointing_to: 'setarch', install_dir: bindir)
endif

