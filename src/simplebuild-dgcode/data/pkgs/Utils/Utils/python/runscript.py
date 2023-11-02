
def create_run_script(filename,cmd, setupsh = None, headerlines=None, extra_prerun_lines = None):
    """Wraps the command in a run script.
    If setupsh is set, it will be sourced from inside the script
    If headerlines are set, they will be added to the top of the script as comments"""
    fh=open(filename,'w')
    fh.write('#!/usr/bin/env bash\n')
    if headerlines:
        for h in headerlines:
            fh.write('#%s\n'%h)
    for extra_line in (extra_prerun_lines or []):
        fh.write('%s\n'%extra_line)
    fh.write('touch state.run && \\\n')
    if setupsh:
        fh.write('source %s && \\\n'%setupsh)
    fh.write('env > env.txt && \\\n')
    import pipes
    fh.write('%s 1>stdout.txt 2>stderr.txt\n'%(' '.join(pipes.quote(a) for a in cmd)))
    fh.write('EC=$?\n')
    fh.write('echo $EC > exitcode.txt\n')
    fh.write('touch state.done\n')
    fh.write('return $EC 2>/dev/null || exit $EC\n')#WAS: fh.write('[ "$0" = "$BASH_SOURCE" ] && exit $EC || return $EC\n')
    fh.close()
