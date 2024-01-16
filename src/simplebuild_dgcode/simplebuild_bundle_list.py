
def simplebuild_bundle_list():
    import pathlib
    datadir = ( pathlib.Path(__file__).absolute().parent
                / 'data' ).absolute().resolve()
    return [ datadir / 'pkgs'     / 'simplebuild.cfg',
             datadir / 'pkgs_val' / 'simplebuild.cfg' ]
