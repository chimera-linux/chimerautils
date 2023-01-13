confd = configuration_data()

confd.set('HAVE_STRTOQ', cc.has_function('strtoq'))
confd.set('HAVE_STRLCPY', cc.has_function('strlcpy'))
confd.set('HAVE_STRLCAT', cc.has_function('strlcat'))
confd.set('HAVE_STRTONUM', cc.has_function('strtonum'))

configure_file(output: 'config.h', configuration: confd)

inc = include_directories('.')
