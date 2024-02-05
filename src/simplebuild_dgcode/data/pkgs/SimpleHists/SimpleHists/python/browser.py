__doc__='module providing a very simple interactive browser for shist files'
__all__=['interactive_browser']

import os

def _terminate(pid):
    #need this workaround since os.killpg(pid, SIGTERM) can give issues on OSX
    #see http://stackoverflow.com/questions/12521705
    import signal
    try:
        os.killpg(pid, signal.SIGTERM)
    except OSError as e:
        if e.errno != 3: # 3 == no such process
            if e.errno == 1:
                try:
                    os.kill(pid, 0)
                except OSError as e2:
                    if e2.errno != 3:
                        print("Error killing %s: %s" %(pid, e))
            else:
                print("Error killing %s: %s" %(pid, e))


_old_highlighted=[]
_sub_procs=[]
def interactive_browser(filename,indices_and_keys=None,dpi=None):
    from SimpleHists._backend_test import _ensure_backend_ok
    _ensure_backend_ok()
    assert dpi is None or (isinstance(dpi,int) and 0<=dpi<=3000)
    try:
        import PySimpleGUI as sg
    except ImportError:
        if 'CONDA_PREFIX' in os.environ:
            msg = ('You might be able to fix this by running the command'
                   ' "conda install pysimplegui" (or as a fallback'
                   ' "python3 -m pip install PySimpleGUI").')
        else:
            msg = ('You might be able to fix this by running the command'
                   ' "python3 -m pip install PySimpleGUI".')
        raise SystemExit(f'ERROR: PySimpleGUI not found. {msg}')
    #sg.theme_previewer()

    import SimpleHists as sh
    import subprocess
    import fnmatch
    import shlex
    hc = sh.HistCollection(filename)
    if not indices_and_keys:
        keys=hc.keys
        indices_and_keys = list(enumerate(keys))
    if not indices_and_keys:
        raise SystemExit("Nothing selected")

    sg.theme('SandyBeach')

    def launch_entry_view(entry,dpi=None):
        global _sub_procs
        cmd=['sb_simplehists_browse','-pq',
             filename,entry.key,f'--dpi={dpi or 0}']
        os.environ['TK_SILENCE_DEPRECATION']='1'#no need to spam repeatedly
        print("Launching command:",' '.join(cmd))
        p = subprocess.Popen([shlex.quote(e) for e in cmd],shell=False,
                             #stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                             preexec_fn=os.setsid)
        _sub_procs+=[p]

    #Fixme: leftover colours from old browser. Keeping here in case PySimpleGUI
    #at some point supports individual item colours in listbox:
    text_col_1d=(0.3,0.3,0.7)
    text_col_2d=(0.3,0.6,0.3)
    text_col_counts=(0.6,0.3,0.3)
    text_hlcol_1d=(0.3,0.3,1.0)
    text_hlcol_2d=(0.3,0.8,0.3)
    text_hlcol_counts=(0.8,0.3,0.3)
    text_highlight_bbox={'facecolor':(0.2,0.2,0.5),  # noqa 841
                         'alpha':0.2,
                         'linewidth':0,
                         #'edgecolor':'black',
                         'boxstyle':'round,pad=0.4'}
    def key2col(k):
        if isinstance(hc.hist(k),sh.Hist1D):
            return text_col_1d
        if isinstance(hc.hist(k),sh.Hist2D):
            return text_col_2d
        return text_col_counts
    def key2hlcol(k):
        if isinstance(hc.hist(k),sh.Hist1D):
            return text_hlcol_1d
        if isinstance(hc.hist(k),sh.Hist2D):
            return text_hlcol_2d
        return text_hlcol_counts

    bn_filename=os.path.basename(filename)

    class Entry:
        def __init__(self,idx,key):
            self.idx = idx
            self.key = key
            self.histtype = str(hc.hist(key).__class__.__name__)
            shorttype = ( self.histtype[4:]
                          if (self.histtype.startswith('Hist')
                              and len(self.histtype)>4)
                          else self.histtype )
            self._str = f'{idx} : {key} [{shorttype}]'
        def __str__(self):
            return self._str
    display_list = [Entry(idx,key) for idx,key in indices_and_keys]

    totalwidth=85

    def determine_fontsize():
        w, h = sg.Window.get_screen_size()
        if w<1024:
            return 14
        if w<1400:
            return 18
        if w<2000:
            return 24
        return 32

    _sh_default_fs = determine_fontsize()
    sh_default_font = ("any", _sh_default_fs)
    sg.set_options(font=sh_default_font)
    sg.set_options(tooltip_font=sh_default_font)

    dpi_choices= [ '<system default>' if not e else str(e)
                   for e in sorted( set([0,dpi or 0]
                                        + list(range(75,201,25))
                                        + list(range(250,1201,50))) ) ]
    dpi_default_idx = 0 if not dpi else dpi_choices.index(str(dpi))
    tooltip_filter='Case-insensitive filter expression. Wildcards allowed.'
    import matplotlib
    tooltip_dpi="""
DPI value used when launching plots. To change the default,
either set the environment variable SIMPLEHISTS_DPI, i.e.
add a line in your ~/.bashrc (or appropriate file):

    export SIMPLEHISTS_DPI=200

Or change DPI for all matplotlib plots by adding the following
line to your global matplotlibrc file:

   figure.dpi=200
""".strip()
    if matplotlib.matplotlib_fname():
        tooltip_dpi +='\n\n'+f"""
On your system, that file is located at:

  {matplotlib.matplotlib_fname()}
""".strip()
    #ca=dict(font='any 30')
    ca={}
    layout = [[sg.Text(f'Browsing SimpleHists file: {bn_filename}',
                       tooltip=filename,**ca)],
              [sg.Listbox(values=display_list, size=(totalwidth, 25),
                          key='-LIST-',
                          enable_events=True,
                          tooltip=('Click on histogram to open in separate'
                                   ' window (press Q to close again).'),
                          **ca)],
              [sg.Text('Filter:', tooltip=tooltip_filter,**ca),
               sg.Input(size=(15,1), key='-filter-',enable_events=True,
                        tooltip=tooltip_filter,**ca),
               sg.Text(' '*12,**ca),
               sg.Text('DPI:',tooltip=tooltip_dpi,font=sh_default_font,**ca),
               sg.Combo(dpi_choices,default_value=dpi_choices[dpi_default_idx],
                        readonly=True,key='-dpi-',tooltip=tooltip_dpi,**ca),
               sg.Text(' '*12,**ca),
               sg.Button('Exit',**ca)]]

    window = sg.Window( f'SimpleHists browser : {bn_filename}',
                        layout,
                        return_keyboard_events=True,
                        auto_size_text = True )
    window.Finalize()
    if hasattr(window,'TKroot') and hasattr(window.TKroot,'attributes'):
        window.TKroot.attributes('-topmost', True)
        window.TKroot.attributes('-topmost', False)

    #Make it super easy to close:
    window.bind('Q','sh_window_close')
    window.bind('q','sh_window_close')
    window.bind('<Control-w>','sh_window_close')
    window.bind('<Control-q>','sh_window_close')
    window.bind('<Control-W>','sh_window_close')
    window.bind('<Control-Q>','sh_window_close')

    while True:
        event, values = window.read()
        if event in ('sh_window_close',sg.WIN_CLOSED, 'Exit','',''):
            break
        elif event == '-filter-':
            if display_list is not None:
                pattern=values['-filter-'].lower().strip()
                if not pattern.endswith('*'):
                    pattern += '*'
                if not pattern.startswith('*'):
                    pattern = '*'+pattern
                new_output = [ ll for ll in display_list
                               if fnmatch.fnmatch(str(ll).lower(),pattern) ]
                window['-LIST-'].update(new_output)
            continue
        elif event=='-LIST-':
            _ = values['-LIST-']
            if _:
                dpi=values['-dpi-']
                dpi = None if 'default' in dpi else int(dpi)
                launch_entry_view(_[0],dpi=dpi)
            continue
        #print ('unused event: ',event,values)

    def kill_children():
        global _sub_procs
        while _sub_procs:
            p=_sub_procs.pop()
            _terminate(p.pid)
    window.close()
    kill_children()

def interactive_plot_hist_from_file(filename,histkey):
    from SimpleHists._backend_test import _ensure_backend_ok
    _ensure_backend_ok()
    import matplotlib.pyplot as plt
    import SimpleHists as sh
    hc=sh.HistCollection(filename)
    hist=hc.hist(histkey)
    hist.plot()
    plt.show()
