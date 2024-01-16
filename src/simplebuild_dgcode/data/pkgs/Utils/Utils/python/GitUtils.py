import os
import pathlib
import subprocess
import Core.System as Sys

#Duplicated from .githooks/hooks.py:
def _strbytes2path(strbytes_path):
    try:
        s = os.fsdecode(strbytes_path) if isinstance(strbytes_path,bytes) else strbytes_path
        fpath = pathlib.Path(s)
        str(fpath).encode('utf8')#check that it can be encoded in utf8 (TODO: Require ASCII?)
    except (UnicodeDecodeError,UnicodeEncodeError):
        raise SystemExit('Forbidden characters in filename detected! : "%s"'%strbytes_path)
    return fpath

#Duplicated from .githooks/hooks.py: We set GIT_CONFIG=/dev/null to avoid
#user/system cfg messing with the result (BUT I AM NOT 100% SURE THIS WORKS). We
#also set GIT_OPTIONAL_LOCKS=0 instead of using --no-optional-locks, since the
#functionality is not available in all git versions we want to support:
_safe_git_cmd = '/usr/bin/env GIT_CONFIG=/dev/null GIT_OPTIONAL_LOCKS=0 git'.split()
_safe_git_cmd_keepcfg = '/usr/bin/env GIT_OPTIONAL_LOCKS=0 git'.split()

#Duplicated from .githooks/hooks.py:
def git_status_iter(extra_args=[],staged_only=False,status_filter=''):
    #We set GIT_CONFIG=/dev/null to avoid user/system cfg messing with the
    #result. We also set GIT_OPTIONAL_LOCKS=0 instead of using
    #--no-optional-locks, since the functionality is not available in all git
    #versions we want to support:
    cmd = _safe_git_cmd + ['diff','--name-status','--no-renames','-z']
    if staged_only:
        cmd += ['--cached']
    if status_filter:
        cmd += ['--diff-filter=%s'%status_filter]
    if extra_args:
        cmd += ['--']
        cmd += [ str(p) for p in extra_args ]
    b=subprocess.run(cmd,check=True, stdout=subprocess.PIPE).stdout
    parts=b.split(b'\x00')
    while parts:
        statusflag=parts.pop(0).decode('ascii')
        if not parts:
            if not statusflag:
                break#guard against trailing empty entry
            raise RuntimeError('Unexpected output of git diff command (%s)'%(cmd))
        fpath = _strbytes2path(parts.pop(0))
        yield statusflag,fpath

def _paths_and_workdirs( specific_paths ):
    #Handle the fact files might be in different git repos:
    for p in specific_paths:
        pp = pathlib.Path(p)
        yield ( pp if pp.is_dir() else pp.parent ), p

def git_untracked_files_iter( specific_paths ):
    cmd = _safe_git_cmd + 'ls-files --others -z -- DUMMY'.split();
    for workdir, checked_path in _paths_and_workdirs( specific_paths ):
        with Sys.changedir(workdir):
            cmd[-1] = str(checked_path)
            b=subprocess.run(cmd,check=True, stdout=subprocess.PIPE).stdout
            for p in b.split(b'\x00'):
                if b:
                    yield _strbytes2path(p)

def git_unclean_files_iter( specific_paths, allow_untracked = False ):
    for workdir, checked_path in _paths_and_workdirs( specific_paths ):
        with Sys.changedir(workdir):
            if not allow_untracked:
                for f in git_untracked_files_iter([checked_path]):
                    yield f
            for _,f in git_status_iter([checked_path]):
                yield f

def is_unclean(specific_paths = None):
    return any(True for f in git_unclean_files_iter(specific_paths))

def abort_if_unclean( specific_paths = None, allow_untracked = False ):
    for f in git_unclean_files_iter(specific_paths,allow_untracked):
        raise SystemExit(f'Aborting since git reports the following unclean file: {f}')

def abort_if_pkg_unclean( pkg_name ):
    from simplebuild.cfg import pkgs
    if is_unclean( [pkgs[pkg_name]['dirname']] ):
        raise SystemExit(f'Aborting since git reports unclean files in the package {pkg_name}')

def git_check_ignore( path ):
    with Sys.changedir( path if path.is_dir() else path.parent ):
        #NB: _keepcfg here, to not skip our gitignore file for this check!
        cmd = _safe_git_cmd_keepcfg + ['check-ignore','-q','--',str(path)]
        ec = subprocess.run(cmd,check=False, stdout=subprocess.PIPE).returncode
        if not ec in (0,1):
            raise RuntimeError("Unexpected error in git check-ignore command")
        return ec==0#ec 0 means "ignored", 1 means "not ignored"

def git_not_ignored( path ):
    return not git_check_ignore( path )

