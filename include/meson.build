confd = configuration_data()

confd.set('HAVE_STRTOQ', cc.has_function('strtoq'))
confd.set('HAVE_STRLCPY', cc.has_function('strlcpy'))
confd.set('HAVE_STRLCAT', cc.has_function('strlcat'))
confd.set('HAVE_STRTONUM', cc.has_function('strtonum'))
confd.set('HAVE_REALLOCF', cc.has_function('reallocf'))
confd.set('HAVE_ERRC', cc.has_function('errc'))
confd.set('HAVE_WARNC', cc.has_function('warnc'))

configure_file(output: 'config-compat.h', configuration: confd)

inc = include_directories('.')
