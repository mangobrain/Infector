datadir = get_option('datadir')

install_data('infector.ui', 'menu.ui',
    install_dir: join_paths(datadir, 'infector'))

i18n.merge_file(
    'desktop',
    input: 'infector.desktop.in',
    output: 'infector.desktop',
    install: true,
    install_dir: join_paths(datadir, 'applications'),
    po_dir: join_paths(meson.source_root(), 'po'),
    type: 'desktop'
)
