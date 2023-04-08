# config.h

nviconf_data = configuration_data()
nviconf_data.set('USE_WIDECHAR', true)
nviconf_data.set('USE_ICONV', true)
nviconf_data.set('ICONV_TRADITIONAL', true)
nviconf_data.set('HAVE_PTY_H', true)

if cc.has_header('ncursesw/ncurses.h', dependencies: ncurses)
    nviconf_data.set('HAVE_NCURSESW_NCURSES_H', true)
elif cc.has_header('ncurses.h', dependencies: ncurses)
    nviconf_data.set('HAVE_NCURSES_H', true)
endif

if cc.has_header('term.h', dependencies: ncurses)
    nviconf_data.set('HAVE_TERM_H', true)
endif

configure_file(
    input: 'files/config.h.in',
    output: 'config.h',
    configuration: nviconf_data
)

# options_def.h

options_def_h = custom_target(
    'options_def_h',
    output: 'options_def.h',
    input: ['common/options.awk', 'common/options.c'],
    command: ['awk', '-f', '@INPUT@'],
    capture: true,
)

# ex_def.h

ex_def_h = custom_target(
    'ex_def_h',
    output: 'ex_def.h',
    input: ['ex/ex.awk', 'ex/ex_cmd.c'],
    command: ['awk', '-f', '@INPUT@'],
    capture: true,
)

# ex/version.h

subdir('ex')

# extern.h

nvi_srcs = [
    # vi curses sources
    'cl/cl_funcs.c', 'cl/cl_main.c', 'cl/cl_read.c',
    'cl/cl_screen.c', 'cl/cl_term.c',
    # ex source
    'ex/ex.c', 'ex/ex_abbrev.c', 'ex/ex_append.c', 'ex/ex_args.c',
    'ex/ex_argv.c', 'ex/ex_at.c', 'ex/ex_bang.c', 'ex/ex_cd.c',
    'ex/ex_cmd.c', 'ex/ex_cscope.c', 'ex/ex_delete.c', 'ex/ex_display.c',
    'ex/ex_edit.c', 'ex/ex_equal.c', 'ex/ex_file.c', 'ex/ex_filter.c',
    'ex/ex_global.c', 'ex/ex_init.c', 'ex/ex_join.c', 'ex/ex_map.c',
    'ex/ex_mark.c', 'ex/ex_mkexrc.c', 'ex/ex_move.c', 'ex/ex_open.c',
    'ex/ex_preserve.c', 'ex/ex_print.c', 'ex/ex_put.c', 'ex/ex_quit.c',
    'ex/ex_read.c', 'ex/ex_screen.c', 'ex/ex_script.c', 'ex/ex_set.c',
    'ex/ex_shell.c', 'ex/ex_shift.c', 'ex/ex_source.c', 'ex/ex_stop.c',
    'ex/ex_subst.c', 'ex/ex_tag.c', 'ex/ex_txt.c', 'ex/ex_undo.c',
    'ex/ex_usage.c', 'ex/ex_util.c', 'ex/ex_version.c', 'ex/ex_visual.c',
    'ex/ex_write.c', 'ex/ex_yank.c', 'ex/ex_z.c',
    # vi source
    'vi/vi.c', 'vi/getc.c', 'vi/v_at.c', 'vi/v_ch.c', 'vi/v_cmd.c',
    'vi/v_delete.c', 'vi/v_ex.c', 'vi/v_increment.c', 'vi/v_init.c',
    'vi/v_itxt.c', 'vi/v_left.c', 'vi/v_mark.c', 'vi/v_match.c',
    'vi/v_paragraph.c', 'vi/v_put.c', 'vi/v_redraw.c', 'vi/v_replace.c',
    'vi/v_right.c', 'vi/v_screen.c', 'vi/v_scroll.c', 'vi/v_search.c',
    'vi/v_section.c', 'vi/v_sentence.c', 'vi/v_status.c', 'vi/v_txt.c',
    'vi/v_ulcase.c', 'vi/v_undo.c', 'vi/v_util.c', 'vi/v_word.c',
    'vi/v_xchar.c', 'vi/v_yank.c', 'vi/v_z.c', 'vi/v_zexit.c',
    # vi screen source
    'vi/vs_line.c', 'vi/vs_msg.c', 'vi/vs_refresh.c', 'vi/vs_relative.c',
    'vi/vs_smap.c', 'vi/vs_split.c',
    # general sources
    'common/conv.c', 'common/cut.c', 'common/delete.c',
    'common/encoding.c', 'common/exf.c', 'common/key.c',
    'common/line.c', 'common/log.c', 'common/main.c',
    'common/mark.c', 'common/msg.c', 'common/options.c',
    'common/options_f.c', 'common/put.c', 'common/recover.c',
    'common/screen.c', 'common/search.c', 'common/seq.c', 'common/util.c',
    # wide char regex
    'regex/regcomp.c', 'regex/regerror.c', 'regex/regexec.c',
    'regex/regfree.c',
]

extern_h = custom_target(
    'extern_h',
    output: 'extern.h',
    input: nvi_srcs,
    command: ['./gen_extern_h.sh', '@INPUT@'],
    capture: true,
)

# pathnames.h

pathnames_data = configuration_data()
pathnames_data.set('vi_cv_path_msgcat', join_paths(
    get_option('prefix'), get_option('datadir'), 'vi/catalog'
) + '/')
pathnames_data.set('vi_cv_path_preserve', '/var/tmp/vi.recover/')

configure_file(
    input: 'files/pathnames.h.in',
    output: 'pathnames.h',
    configuration: pathnames_data
)

configure_file(
    input: 'files/recover.in',
    output: 'recover',
    configuration: pathnames_data,
    install: true,
    install_dir: get_option('datadir') / 'vi',
    install_mode: 'rwxr-xr-x',
)

# actual build

# need _XOPEN_SOURCE=600 for some ncurses functions
extra_cargs = ['-D__REGEX_PRIVATE', '-D_XOPEN_SOURCE=600']

if cc.has_argument('-Wunused-but-set-variable')
    extra_cargs += ['-Wno-unused-but-set-variable']
endif

if cc.has_argument('-Wunused-parameter')
    extra_cargs += ['-Wno-unused-parameter']
endif

if cc.has_argument('-Wclobbered')
    extra_cargs += ['-Wno-clobbered']
endif

if cc.has_argument('-Wsign-compare')
    extra_cargs += ['-Wno-sign-compare']
endif

if cc.has_argument('-Wmissing-field-initializers')
    extra_cargs += ['-Wno-missing-field-initializers']
endif

nvi_prog = executable(
    'vi',
    nvi_srcs + [ options_def_h, ex_def_h, ex_version_h, extern_h ],
    include_directories: [include_directories('.', 'regex'), inc, cinc, dbinc],
    dependencies: [ncurses],
    link_with: [libcompat, libdbcompat],
    c_args: extra_cargs,
    install: true,
)

# TODO: install catalog (localization)

# links
install_symlink('ex', pointing_to: 'vi', install_dir: bindir)
install_symlink('nex', pointing_to: 'vi', install_dir: bindir)
install_symlink('view', pointing_to: 'vi', install_dir: bindir)
install_symlink('nview', pointing_to: 'vi', install_dir: bindir)
install_symlink('nvi', pointing_to: 'vi', install_dir: bindir)

# manpage
install_man('man/vi.1')
install_symlink('ex.1', pointing_to: 'vi.1', install_dir: man1dir)
install_symlink('nex.1', pointing_to: 'vi.1', install_dir: man1dir)
install_symlink('view.1', pointing_to: 'vi.1', install_dir: man1dir)
install_symlink('nview.1', pointing_to: 'vi.1', install_dir: man1dir)
install_symlink('nvi.1', pointing_to: 'vi.1', install_dir: man1dir)
