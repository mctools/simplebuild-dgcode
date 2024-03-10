#!/usr/bin/env python3
import pathlib

def determine_default_project_dir():
    from _simple_build_system.envcfg import var as sbcfg
    assert sbcfg.main_bundle_pkg_root.is_dir()
    return sbcfg.main_bundle_pkg_root

def get_target_dir(target_dir, project_name):
    return ( target_dir
             if target_dir
             else determine_default_project_dir() / project_name )

def parse_cmdline():
    from ._utils import normpath, normpath_or_none, is_empty_dir
    import argparse

    descr=('Creates, and populates with skeleton code,'
           ' new packages for a simulation project')
    parser = argparse.ArgumentParser(description=descr.strip())
    parser.add_argument('project_name', metavar='PROJECTNAME', type=str,
                        help=('Project name. Should be short and'
                              ' descriptive CamelCased string with no spaces'))
    parser.add_argument("-d", "--target_dir",metavar='DIR',
                        type = normpath_or_none, default=None,
                        help=('Target directory. Defaults to <root>/PROJECTNAME'
                              ' where <root> is the pkg-root associated with '
                              'your main simplebuild.cfg file.'))
    args=parser.parse_args()
    if not args.project_name:
        parser.error('Must supply project name')
    try:
        args.project_name.encode('ascii')
    except UnicodeEncodeError:
        parser.error('Invalid characters in project name')
    if not args.project_name[0].isupper():
        parser.error('Project name must begin with upper case'
                     ' letter (and be CamelCased)')
    if not args.project_name.isalnum():
        parser.error('Only alpha numeric characters'
                     ' should be used in the project name')
    if len(args.project_name)<3:
        parser.error('Project name too short.')
    tdir = get_target_dir(args.target_dir, args.project_name)
    if tdir.exists() and not is_empty_dir(tdir):
        parser.error(f'Destination directory already exists: {normpath(tdir)}')
    #if args.project_name in cfg.pkgs:
    #    parser.error(f'Project name already taken: {args.project_name}')
    return args


def load_skeleton_vfile_provider():
    skeldir = pathlib.Path(__file__).parent / 'data' / 'pkgs_val' / 'SkeletonSP'
    assert skeldir.is_dir()
    assert ( skeldir / 'G4GeoSkeletonSP' / 'pkg.info' ).is_file()
    from ._utils import recursive_vfile_iter
    return recursive_vfile_iter( skeldir )

class ProjectFileProvider:
    def __init__(self,projectname):

        self._srciter = load_skeleton_vfile_provider()
        skeltxt = ( 'After you have modified this file as needed for your '
                    'project you can remove this line and commit <NOCOM'
                    'MIT>' )
        self._map = list(sorted(set([('SkeletonSP',projectname),
                                     ('SKELETONSP',projectname.upper()),
                                     ('skeletonsp',projectname.lower()),
                                     ('<SKEL_MUST_MODIFY_FILE>',skeltxt)
                                     ])))
    def _mapstr(self, s):
        for k,v in self._map:
            if k in s:
                s = s.replace(k,v)
        return s

    def __iter__(self):
        for vfile in self._srciter:
            assert vfile.is_dir or vfile.content is not None
            path = pathlib.Path( *list(self._mapstr(p)
                                       for p in vfile.path.parts) )
            if vfile.is_dir or isinstance(vfile.content,bytes):
                yield vfile.modified( path=path )
            else:
                yield vfile.modified( path=path,
                                      content = self._mapstr(vfile.content) )


def main( argv ):
    args = parse_cmdline()
    target_dir = get_target_dir(args.target_dir, args.project_name)

    if target_dir.exists():
        raise SystemExit(f'ERROR: directory already exists: {target_dir}')
    target_dir.mkdir(parents=True)
    def write_callbackfct(p):
        print(f'Created file: {p.relative_to(target_dir.parent)}')

    from ._utils import write_vfiles_under_dir
    n = write_vfiles_under_dir( ProjectFileProvider(args.project_name),
                                target_dir,
                                write_callbackfct = write_callbackfct
                               )

    text=f"""
Created {n} new files from the simulation project skeleton under

        {target_dir}

Now you can go carefully through them and replace their contents as needed for
your project.

Do not forget to update documentation in comments and pkg.info files!
"""

    text += """
Make sure that you have edited all files, and that everything is tested with at
least "sb --tests" before committing anything to a shared repository!
""".rstrip()+'\n'
    print(text)

if __name__ == '__main__':
    import sys
    main( sys.argv )
