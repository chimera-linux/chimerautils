confd = configuration_data()

confd.set('PROJECT_NAME', '"' + meson.project_name() + '"')
confd.set('PROJECT_VERSION', '"' + meson.project_version() + '"')

confd.set_quoted('LIBEXECDIR', join_paths(get_option('prefix'), get_option('libexecdir')))

confd.set('HAVE_STRTOQ', cc.has_function('strtoq'))
confd.set('HAVE_STRLCPY', cc.has_function('strlcpy'))
confd.set('HAVE_STRLCAT', cc.has_function('strlcat'))
confd.set('HAVE_STRTONUM', cc.has_function('strtonum'))
confd.set('HAVE_REALLOCF', cc.has_function('reallocf'))
confd.set('HAVE_ERRC', cc.has_function('errc'))
confd.set('HAVE_WARNC', cc.has_function('warnc'))
confd.set('HAVE_FTS_OPEN', cc.has_function('fts_open'))
confd.set('HAVE_RPMATCH', cc.has_function('rpmatch'))
confd.set('HAVE_PAM_MISC', pam_misc.found())

configure_file(output: 'config-compat.h', configuration: confd)

inc = include_directories('.')
