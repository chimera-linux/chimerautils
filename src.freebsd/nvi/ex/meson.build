# ex/version.h

ex_version_h = custom_target(
    'ex_version_h',
    output: 'version.h',
    input: '../README',
    command: ['../gen_ex_version_h.sh', '@INPUT@'],
    capture: true,
)
