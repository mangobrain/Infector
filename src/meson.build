gtkmm = dependency('gtkmm-3.0', version: '>=3.22')

cfg = configuration_data()

if target_machine.system() == 'windows'
    cfg.set_quoted('INFECTOR_PKGDATADIR', '../share/infector')
    cfg.set_quoted('INFECTOR_LOCALEDIR', '../share/locale')
else
    cfg.set_quoted('INFECTOR_PKGDATADIR', join_paths(get_option('prefix'),
                get_option('datadir'), 'infector'))
    cfg.set_quoted('INFECTOR_LOCALEDIR', join_paths(get_option('prefix'),
                get_option('localedir')))
endif

if get_option('enable_nls')
    cfg.set('ENABLE_NLS', true)
    cfg.set_quoted('GETTEXT_PACKAGE', 'infector')
endif

configure_file(output: 'config.h', configuration: cfg)

exe = executable('infector',
    'ai.cxx', 'boardstate.cxx', 'clientstatusdialog.cxx', 'gameboard.cxx',
    'game.cxx','infector.cxx', 'newgamedialog.cxx', 'serverstatusdialog.cxx',
    'socket.cxx',
    dependencies: gtkmm,
    install: true
)
