import pathlib
import os
import re
import shlex
import subprocess
from _simple_build_system._sphinxutils import _get_gitversion

 # Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

project = 'simplebuild-dgcode'
copyright = '2013-2024, ESS ERIC and simplebuild developers'
author = 'Thomas Kittelmann'

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

nitpicky = True
extensions = [
    # 'myst_parser', #for parsing .md files?
    'sphinxarg.ext',
    'sphinx_toolbox.sidebar_links',
    'sphinx_toolbox.github',
    'sphinx_toolbox.collapse',
    #'sphinx_licenseinfo',

    #Issue roles (pip install sphinx-issues,
    #             docs at https://github.com/sloria/sphinx-issues):
    #'sphinx_issues',
    ]


#Fix env:
os.environ['PYTHONUNBUFFERED'] = '1'
if 'SIMPLEBUILD_CFG' in os.environ:
    del os.environ['SIMPLEBUILD_CFG']

def _reporoot():
    p=pathlib.Path(__file__).parent.parent.parent
    assert ( p / 'src' / 'simplebuild_dgcode'/ '__init__.py' ).is_file()
    return p.absolute()

version = _get_gitversion( _reporoot() )

# for :sbpkg:`MyPkg` links
extensions += '_simple_build_system._sphinxext',
def _sbbundles():
    #For now, just linking to main branch on github. This could be improved!
    sbdgversion='main'
    if re.match(r"^v[0-9]+\.[0-9]+\.[0-9]+$", version ):
        sbdgversion = version
    sbversion='main' # probably is best to keep at main
    sbdgdata = _reporoot() / 'src' / 'simplebuild_dgcode' / 'data'
    import _simple_build_system as _
    sbdata = pathlib.Path(_.__file__).parent / 'data'
    sbdgdata_online = f'https://github.com/mctools/simplebuild-dgcode/tree/{sbdgversion}/src/simplebuild-dgcode/data'#FIXME FIXME url "-" to "_"!!!
    sbdata_online = f'https://github.com/mctools/simplebuild/tree/{sbversion}/src/_simple_build_system/data'

    bundles = { 'dgcode' : ( sbdgdata / 'pkgs', sbdgdata_online + '/pkgs' ),
                'dgcode_val' : ( sbdgdata / 'pkgs_val', sbdgdata_online + '/pkgs_val' ),
                'core' :  ( sbdata / 'pkgs-core', sbdata_online + '/pkgs-core' ),
                'core_val' : ( sbdata / 'pkgs-core_val', sbdata_online + '/pkgs-core_val' ),
               }
    return bundles
simplebuild_pkgbundles = _sbbundles()

#https://img.shields.io/github/issues/mctools/simplebuild
#https://img.shields.io/github/issues/mctools/simplebuild/bug
#https://img.shields.io/github/issues/detail/state/mctools/simplebuild/36
#https://img.shields.io/github/issues/detail/state/mctools/simplebuild/41

#For the toolbox.sidebar_links:
github_username = 'mctools'
github_repository = 'simplebuild-dgcode'

#For the 'sphinx_issues' extension:
issues_default_group_project = f"{github_username}/{github_repository}"

templates_path = ['_templates']
exclude_patterns = []

# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output #noqa E501

html_theme = 'sphinx_rtd_theme'
html_static_path = ['_static']
#html_logo = 'icon/favicon-32x32.png'
html_favicon = 'icon/favicon-32x32.png'
#html_sidebars = { '**': ['globaltoc.html', 'searchbox.html'] }

html_theme_options = {
    'logo_only': False,
#    'collapse_navigation' : False,
    'sticky_navigation': True, #sidebar stays in place while contents scrolls
    'navigation_with_keys': True, #navigate with left/right arrow keys
    #'github_url' : 'https://github.com/mctools/simplebuild',
}

#html_theme_options = {
#    'logo': 'icon/android-chrome-192x192.png',
#    # etc.
#}
#

#html_sidebars = { '**': ['globaltoc.html', 'relations.html',
#                         'sourcelink.html', 'searchbox.html'] }

#html_theme_options = {
#    ...
#    "home_page_in_toc": True
#    ...
#}
#

###############################################################################

def prepare_tricorderproj_dir():
    confpydir = pathlib.Path(__file__).parent
    blddir = confpydir.parent / 'build'
    blddir.mkdir(exist_ok=True)
    assert blddir.exists()
    root = blddir / 'autogen_tricorder_projdir'
    already_done = root.is_dir()
    if not already_done:
        root.mkdir()
    class dirs:
        pass
    d = dirs()
    d.root = root
    d.blddir = blddir
    d.confpydir = confpydir
    d.already_done = already_done
    return d

def invoke( *args, **kwargs ):
    assert len(args)>0
    cmd = args[0]
    cmdstr = shlex.join( cmd )
    orig_check = kwargs.get('check')
    if 'check' in kwargs:
        del kwargs['check']
    cwd = kwargs.get('cwd')
    assert cwd
    orig_capture_output = kwargs.get('capture_output')
    kwargs['capture_output'] = True
    p = subprocess.run( *args, **kwargs )
    if orig_check:
        if p.returncode != 0 or p.stderr:
            print(p.stderr.decode())
            print(p.stdout.decode())
            raise RuntimeError(f'Command in dir {cwd} failed!: {cmdstr}')
    if p.stderr and (orig_check or orig_capture_output ):
            print(p.stderr.decode())
            print(p.stdout.decode())
            raise RuntimeError(f'Command in dir {cwd} had stderr output!: {cmdstr}')
    return p

def invoke_in_pkgroot( cmd, pkgroot, outfile ):
    cmdstr = ' '.join( shlex.quote(e) for e in cmd )
    print(f' ---> Launching command {cmdstr}')
    if cmd[0].startswith('sb_'):
        cmd = ['sbenv']+cmd
    p = invoke( cmd,
                cwd = pkgroot,
                check = True )
    assert not p.stderr
    txt = p.stdout.decode()
    from _simple_build_system._sphinxutils import fixuptext
    txt = fixuptext( pkgroot, p.stdout.decode() )
    txt = f'$> {cmdstr}\n' + txt
    outfile.write_text(txt)

def run_tricorder_cmds():
    tricorder_dirs = prepare_tricorderproj_dir()
    pkgroot = tricorder_dirs.root
    bd = tricorder_dirs.blddir
    c0 = bd / 'autogen_tricorder_sbinit.txt'
    c1 = bd / 'autogen_tricorder_newsimproj_sb0.txt'
    if tricorder_dirs.already_done:
        print(">> Skipping tricorder_cmds (already ran).")
    else:
        invoke_in_pkgroot( ['sb','--init','dgcode'],
                           pkgroot,
                           c0 )
        invoke_in_pkgroot( ['sb'],
                           pkgroot,
                           c1 )
        invoke_in_pkgroot( ['dgcode_newsimproject','-h'],
                           pkgroot,
                           bd / 'autogen_tricorder_newsimproj_help.txt' )
        invoke_in_pkgroot( ['dgcode_newsimproject','TriCorder'],
                           pkgroot,
                           bd / 'autogen_tricorder_newsimproj_TriCorder.txt' )
        invoke_in_pkgroot( ['sb','--tests'],
                           pkgroot,
                           bd / 'autogen_tricorder_newsimproj_sbtests.txt' )
    edit = c0.read_text()
    ll = c1.read_text().splitlines()
    while ll[0].startswith('$>'):
        edit += ll[0]+'\n'
        ll = ll[1:]
    ntop,nbottom=5,5
    for i,line in enumerate(ll):
        if i < ntop or i >= len(ll)-nbottom:
            edit += line.rstrip()+'\n'
        elif i==ntop:
            nstripped = len(ll)-ntop-nbottom
            edit += ( f'<<\n<< {nstripped} LINES OF ACTUAL BUILD OUTPUT'
                      ' NOT SHOWN HERE >>\n<<\n' )
    (c0.parent
     /c0.name.replace('.txt','_plus_snippet_sb.txt')).write_text(edit)

run_tricorder_cmds()
