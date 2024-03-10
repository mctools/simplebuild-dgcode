
def create_run_script(filename,cmd, simplebuildcfg = None, headerlines=None, extra_prerun_lines = None):
    """Wraps the command in a run script.
    If simplebuildcfg is set, the command will set SIMPLEBUILD_CFG to that inside the script
    If headerlines are set, they will be added to the top of the script as comments"""
    import shlex
    if simplebuildcfg is None:
        import _simple_build_system.cfglocate as _
        simplebuildcfg = _.locate_main_cfg_file()

    fh=open(filename,'w')
    fh.write('#!/usr/bin/env bash\n')
    if headerlines:
        for h in headerlines:
            fh.write('#%s\n'%h)
    for extra_line in (extra_prerun_lines or []):
        fh.write('%s\n'%extra_line)
    fh.write('touch state.run && \\\n')
    fh.write('export SIMPLEBUILD_CFG=%s && \\\n'%shlex.quote(str(simplebuildcfg)))
    fh.write('eval "$(sb --env-setup)" && \\\n')
    fh.write('env > env.txt && \\\n')
    fh.write('%s 1>stdout.txt 2>stderr.txt\n'%(' '.join(shlex.quote(a) for a in cmd)))
    fh.write('EC=$?\n')
    fh.write('echo $EC > exitcode.txt\n')
    fh.write('touch state.done\n')
    fh.write('return $EC 2>/dev/null || exit $EC\n')
    fh.close()
