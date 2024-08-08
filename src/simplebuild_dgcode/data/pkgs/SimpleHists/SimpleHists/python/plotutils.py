from functools import reduce

__doc__='module for plotting SimpleHists'
__all__=['plot1d','plot2d','plot2d_lego','plotcounts','plot1d_overlay','overlay']

#TODO: If we decide to suppress some warnings emitted through the warnings module, we could use:
#import warnings
#with warnings.catch_warnings():
#    #import but suppress warning: 'Matplotlib is building the font cache using fc-list. This may take a moment.'
#    warnings.filterwarnings("ignore",message='.*font.*cache.*fc-list.*')
#    import SimpleHists._backend_test
#    import matplotlib.patches as mplp
#    import matplotlib.pyplot as plt

from SimpleHists._backend_test import _ensure_backend_ok
import matplotlib
import matplotlib.patches as mplp
import matplotlib.pyplot as plt
_interactive = not (matplotlib.get_backend().lower()=='agg')

def _col(cmap,i,n):
    assert i<n
    return cmap(i/(n-1.0)) if n>1 else cmap(1.0)

#unused shortcuts: juz + numbers + up/down arrows

_help_text="""
Standard (matplotlib):

  Home/Reset         : h or r or home
  Back               : c or left arrow or backspace
  Forward            : v or right arrow
  Pan/Zoom           : p
  Zoom-to-rect       : o
  Save               : CTRL+s
  Toggle fullscreen  : CTRL+f
  Close plot         : CTRL+w
  Constrain pan/zoom to x axis     : hold x when panning/zooming with mouse
  Constrain pan/zoom to y axis     : hold y when panning/zooming with mouse
  Preserve aspect ratio            : hold CTRL when panning/zooming with mouse
  Toggle grid                      : g when mouse is over an axes
  Toggle x axis scale (log/linear) : L or k when mouse is over an axes[1DONLY]
  Toggle y axis scale (log/linear) : l when mouse is over an axes[1DONLY]
  Toggle y axis scale (log/linear) : l when mouse is over an axes[COUNTSONLY]

Extra (SimpleHists.Hist1D):[1DONLY]
Extra (SimpleHists.Hist2D):[2DONLY]
Extra (SimpleHists.HistCounts):[COUNTSONLY]

  Close plot     : q
  This info page : i or CTRL+h or F1 or ESC
  Colormap       : m (next), n (previous), t (turn)[2DONLY]
  Interpolation  : a[2DONLY]
  Bar color      : m (next), n (previous)[1DONLY]
  Bar color      : m (next), n (previous)[COUNTSONLY]
  Bar edges      : t[1DONLY]
  Bar edges      : t[COUNTSONLY]
  Bar/error mode : a[1DONLY]
  Bar/error mode : a[COUNTSONLY]
  Error style    : e[1DONLY]
  Error style    : e[COUNTSONLY]
  Lego plot      : d[2DONLY]
  StatBox toggle : b
  Normalise      : 1 (all) / CTRL+1 (inside bin range)[1DONLY]
  Normalise      : 1 (all) / CTRL+1 (inside bin range)[2DONLY]
  Normalise      : 1 [COUNTSONLY]
  Rebin          : 2 [1DONLY]
  Open new copy  : u
"""
_help_text=_help_text.split('\n')
_help_text_width=max(len(a) for a in _help_text)

def _add_help_text(fig,ax,histtype):
    if histtype==1:
        block,select=['[2DONLY]','[COUNTSONLY]'],'[1DONLY]'
    elif histtype==2:
        block,select=['[1DONLY]','[COUNTSONLY]'],'[2DONLY]'
    elif histtype==3:
        block,select=['[1DONLY]','[2DONLY]'],'[COUNTSONLY]'
    else:
        assert False

    ht=[]
    for ll in _help_text:
        ok=True
        for b in block:
            if b in ll:
                ok=False
        if ok:
            ht+=[ ll.replace(select,'') ]
    t=('\n '.join(ht)).strip()
    ib = ax.text(0.5,0.5,t,transform=ax.transAxes,
                 verticalalignment='center',horizontalalignment='center',multialignment='left',
                 bbox={'facecolor':(0.8,0.8,1.0),'alpha':1.0,'edgecolor':'black', 'boxstyle':'round,pad=1'},
                 family='monospace',picker=True)
    #NB: Move to figure to avoid colorbar overlapping in 2d plots (colorbars
    #reside in different subplots):
    if hasattr(ax.texts,'remove'):
        ax.texts.remove(ib)
    ib.figure = fig
    fig.texts.append(ib)
    ib.set_visible(False)
    ib.set_zorder(99999)#big number, one bigger than for statbox
    _resize_text_reltofig(fig,ib,0.8)
    return ib

def _add_statbox(fig,ax,stats,vis,snap_to_corner=False):
    statbox_text = formatStatsText(stats)
    pad=15
    sb = ax.text(1.0,1.0,statbox_text,transform=ax.transAxes,
                 verticalalignment='top',horizontalalignment='right',multialignment='left',
                 bbox={'facecolor':'white', 'edgecolor':'black','alpha':1.0, 'pad':pad},
                 family='monospace',picker=True)

    if not snap_to_corner:
        sb.statbox_corner_pos = (1,1)
    else:
        def sb_first_draw(evt):
            assert pad==15
            pad_pixels=evt.canvas.get_renderer().points_to_pixels(pad)#apparently just multiplies with fig.get_dpi()/72. ~=1.1111
            p1=ax.transAxes.inverted().transform((pad_pixels*0.5,0.5*pad_pixels))
            p0=ax.transAxes.inverted().transform((0,0))
            sb.statbox_corner_pos=(1-p1[0]+p0[0],1-p1[1]+p0[1])
            sb.set_position(sb.statbox_corner_pos)
            fig.canvas.mpl_disconnect(sb_first_draw._the_cid)
            fig.canvas.draw()
        sb_first_draw._the_cid=fig.canvas.mpl_connect('draw_event', sb_first_draw)
        def sb_resize_evt(evt):
            if not evt.canvas.get_renderer():
                return
            assert pad==15
            sbp=sb.get_position()
            pad_pixels=evt.canvas.get_renderer().points_to_pixels(pad)#apparently just multiplies with fig.get_dpi()/72. ~=1.1111
            p1=ax.transAxes.inverted().transform((pad_pixels*0.5,0.5*pad_pixels))
            p0=ax.transAxes.inverted().transform((0,0))
            if hasattr(sb,'statbox_corner_pos'):
                in_corner = (abs(sbp[0]-sb.statbox_corner_pos[0])<1.0e-9 and abs(sbp[1]-sb.statbox_corner_pos[1])<1.0e-9)
            else:
                in_corner = True
            sb.statbox_corner_pos=(1-p1[0]+p0[0],1-p1[1]+p0[1])
            if in_corner:
                sb.set_position(sb.statbox_corner_pos)
                evt.canvas.draw()
        fig.canvas.mpl_connect("resize_event", sb_resize_evt)

    #FIXME: This could perhaps be done easier with
    #offset = transforms.ScaledTranslation(dx, dy,fig.dpi_scale_trans)
    #shadow_transform = ax.transData + offset

    #NB: Move to figure to avoid colorbar overlapping in 2d plots (colorbars
    #reside in different subplots):
    if hasattr(ax.texts,'remove'):
        ax.texts.remove(sb)
    sb.figure = fig
    fig.texts.append(sb)
    sb.set_visible(vis)
    sb.set_zorder(99998)#big number, one smaller than for infobox
    _resize_text_reltofig(fig,sb,0.33)
    return sb

def _resize_text_reltofig(figure,textobject,maxsize=0.8):
    currently_resizing = set()
    def do_resize(canvas):
        if canvas in currently_resizing:
            return#prevent triggering recursion
        if textobject.figure is None:
            return#Avoid crash in textobject.get_window_extent call
        currently_resizing.add(canvas)
        width,height=canvas.get_width_height()
        vis = textobject.get_visible()
        if not vis:
            textobject.set_visible(True)
        textobject.set_size(12.0)
        bb = textobject.get_window_extent(renderer=canvas.get_renderer())
        oversize = max(bb.width/float(width),bb.height/float(height))
        target_size=min(12,textobject.get_size()*maxsize/float(oversize))
        textobject.set_size(target_size)
        if not vis:
            textobject.set_visible(False)
        else:
            canvas.draw()
        currently_resizing.remove(canvas)

    def handle_firstdraw(evt):
        do_resize(evt.canvas)
        figure.canvas.mpl_disconnect(cid_firstdraw)
    def handle_resize(evt):
        do_resize(evt.canvas)
    cid_firstdraw=figure.canvas.mpl_connect("draw_event", handle_firstdraw)
    figure.canvas.mpl_connect("resize_event", handle_resize)

def _hovertext_setpos(handler,evt):
    assert handler._text
    handler._text.set_x(evt.xdata)
    handler._text.set_y(evt.ydata)
    xfig,yfig=handler._fig.transFigure.inverted().transform((evt.x,evt.y))
    if handler._text_prev_xfig is None or bool(xfig<0.5)!=bool(handler._text_prev_xfig<0.5):
        handler._text_prev_xfig=xfig
        handler._text.set_horizontalalignment('left' if xfig<0.5 else 'right')
    if handler._text_prev_yfig is None or bool(yfig<0.5)!=bool(handler._text_prev_yfig<0.5):
        handler._text_prev_yfig=yfig
        handler._text.set_verticalalignment('bottom' if yfig<0.5 else 'top')

def _fixbin(ibin,x,xmin,xmax,binwidth,nbins):
    #work-around floating point issue
    if ibin==-1 and x + binwidth * 1.0e-6 > xmin:
        return 0
    if ibin==nbins and x - binwidth * 1.0e-6 < xmax:
        return nbins-1
    return ibin

class _hoverhandler1d:

    def __init__(self,figure,axes,hist1d):
        figure.canvas.mpl_connect("motion_notify_event", self.on_move)
        self._fig=figure
        self._h=hist1d
        self._axes=axes
        self._rect=None
        self._text=None
        self._text_prev_xfig=None
        self._text_prev_yfig=None
        self._rect_bin=None

    def _hovertext(self,ibin):
        return u'bin    : %i (%g \u2264 x %s %g)\ncontent: %g \u00b1 %g'%(ibin,
                                                                      self._h.getBinLower(ibin),
                                                                      u'\u2264' if ibin+1==self._h.nbins else '<',
                                                                      self._h.getBinUpper(ibin),
                                                                      self._h.getBinContent(ibin),
                                                                      self._h.getBinError(ibin))

    def _update_rect_location(self,evt,ibin):
        assert ibin>=0
        assert ibin<self._h.nbins
        ymin=1.0e-99#not 0 => it will work for log plots as well
        if not self._rect:
            self._rect_bin=ibin
            self._rect=mplp.Rectangle((self._h.getBinLower(ibin),ymin), self._h.binwidth,self._h.maxcontent*100+100, alpha=0.1,color='b')
            self._text = evt.inaxes.text(evt.xdata,evt.ydata,self._hovertext(ibin),
                                         verticalalignment='bottom',horizontalalignment='left',multialignment='left',
                                         bbox={'facecolor':(0.8,0.8,1.0),'alpha':0.5,'edgecolor':'black', 'boxstyle':'round,pad=0.5'},
                                         family='monospace')
            _resize_text_reltofig(self._fig,self._text,0.4)
            evt.inaxes.add_patch(self._rect)
            _hovertext_setpos(self,evt)
            evt.canvas.draw()
            return
        assert self._rect and self._text and self._rect_bin is not None
        _hovertext_setpos(self,evt)
        if self._rect_bin==ibin:
            if not self._rect.get_visible():
                self._rect.set_visible(True)
                self._text.set_visible(True)
            evt.canvas.draw()
            return
        self._rect.set_xy((self._h.getBinLower(ibin),ymin))
        self._rect_bin=ibin
        self._text.set_text(self._hovertext(ibin))
        evt.canvas.draw()

    def on_move(self,evt):
        if not evt or self._axes!=evt.inaxes:
            if self._rect and self._rect.get_visible():
                self._rect.set_visible(False)
                self._text.set_visible(False)
                evt.canvas.draw()
            return
        ibin=self._h.valueToBin(evt.xdata)
        ibin=_fixbin(ibin,evt.xdata,self._h.xmin,self._h.xmax,self._h.binwidth,self._h.nbins)
        if not (0 <= ibin < self._h.nbins):
            print("Hist1D plot WARNING: invalid ibin encountered (should not happen)")
            if self._rect and self._rect.get_visible():
                self._rect.set_visible(False)
                evt.canvas.draw()
        self._update_rect_location(evt,ibin)

class _hoverhandleroverlay:

    def __init__(self,figure,axes,lines,labels):
        figure.canvas.mpl_connect("motion_notify_event", self.on_move)
        self._fig=figure
        self._axes=axes
        self._lines=lines
        self._labels=labels
        self._text=None
        self._text_prev_xfig=None
        self._text_prev_yfig=None
        self._old_text_content=None

    def _update_hover(self,evt,text_content=None):
        if not text_content:
            #hide if shown
            if self._text and self._text.get_visible():
                self._text.set_visible(False)
                evt.canvas.draw()
            return
        if not self._text:
            #First time:
            self._text = evt.inaxes.text(evt.xdata,evt.ydata,text_content,
                                         va='bottom',ha='left',ma='left',
                                         bbox={'facecolor':(0.8,0.8,1.0),'alpha':0.5,'edgecolor':'black', 'boxstyle':'round,pad=0.5'},
                                         family='monospace')
            _resize_text_reltofig(self._fig,self._text,0.4)
            self._old_text_content=text_content
        elif self._old_text_content != text_content:
            self._text.set_text(text_content)
            self._old_text_content=text_content
        if not self._text.get_visible():
            self._text.set_visible(True)
        _hovertext_setpos(self,evt)
        evt.canvas.draw()

    def on_move(self,evt):
        if not evt or self._axes!=evt.inaxes:
            self._update_hover(evt)#hide
            return

        #Are we over a line?
        iline=None
        for i,lset in enumerate(self._lines):
            for ll in lset:
                ll.set_pickradius(2)
                c=ll.contains(evt)
                if c[0]:
                    if iline is None:
                        iline=i
                        break
                    else:
                        self._update_hover(evt)#multiple lines selected, hide
                        return
        if iline is not None:
            self._update_hover(evt,self._labels[iline])
        else:
            self._update_hover(evt)#hide

def _wrap(text, width):
    """
    A word-wrap function that preserves existing line breaks
    and most spaces in the text. Expects that existing line
    breaks are posix newlines (\n).
    """
    #From http://code.activestate.com/recipes/148061-one-liner-word-wrap-function/
    return reduce(lambda line, word, width=width: '%s%s%s' %
                  (line,
                   ' \n'[(len(line)-line.rfind('\n')-1
                         + len(word.split('\n',1)[0]
                              ) >= width)],
                   word),
                  text.split(' ')
                 )

class _hoverhandlercounts:

    def __init__(self,figure,axes,histcounts):
        figure.canvas.mpl_connect("motion_notify_event", self.on_move)
        self._width=1.0#FIXME HARDCODED
        self._spacing=0.5#FIXME HARDCODED
        self._fig=figure
        self._h=histcounts
        self._counters=histcounts.counters
        self._ncounters=len(self._counters)
        self._axes=axes
        self._rect=None
        self._text=None
        self._text_prev_xfig=None
        self._text_prev_yfig=None
        self._rect_bin=None

    def _hovertext(self,icounter):
        c = self._counters[icounter]
        t = u'display label : %s\naccess label  : %s\ncounts        : %g \u00b1 %g'%(c.displaylabel,c.label,c.value,c.error)
        ig = self._h.getIntegral()
        if ig:
            #Simple error scaling assumes small errors and no uncertainty on the integral itself!
            t+=u'\nrelative      : %g \u00b1 %g %%'%(c.value*100.0/ig,c.error*100.0/ig)
        if c.comment:
            t+='\ncomment: %s'%(_wrap(c.comment,40).replace('\n','\n         '))
        return t

    def _update_rect_location(self,evt,icounter):
        w = self._width + self._spacing
        left = icounter*w
        #,right = icounter*w+self._width
        ymin=1.0e-99#not 0 => it will work for log plots as well
        if not self._rect:
            self._rect_bin=icounter
            self._rect=mplp.Rectangle((left,ymin), self._width,self._h.maxcontent*100+100, alpha=0.1,color='b')
            self._text = evt.inaxes.text(evt.xdata,evt.ydata,self._hovertext(icounter),
                                         verticalalignment='bottom',horizontalalignment='left',multialignment='left',
                                         bbox={'facecolor':(0.8,0.8,1.0),'alpha':0.5,'edgecolor':'black', 'boxstyle':'round,pad=0.5'},
                                         family='monospace')
            _resize_text_reltofig(self._fig,self._text,0.5)
            evt.inaxes.add_patch(self._rect)
            _hovertext_setpos(self,evt)
            evt.canvas.draw()
            return
        assert self._rect and self._text and self._rect_bin is not None
        _hovertext_setpos(self,evt)
        if self._rect_bin==icounter:
            if not self._rect.get_visible():
                self._rect.set_visible(True)
                self._text.set_visible(True)
            evt.canvas.draw()
            return
        self._rect.set_xy((left,ymin))
        self._rect_bin=icounter
        self._text.set_text(self._hovertext(icounter))
        evt.canvas.draw()

    def on_move(self,evt):
        w=self._width+self._spacing
        if not evt or self._axes!=evt.inaxes or evt.xdata<0 or evt.xdata>w*self._ncounters or evt.xdata%w>self._width:
            if self._rect and self._rect.get_visible():
                self._rect.set_visible(False)
                self._text.set_visible(False)
                evt.canvas.draw()
            return

        icounter = int(evt.xdata/1.5)
        #workaround floating point issues:
        if icounter==-1 and evt.xdata > -1.0e-6:
            icounter = 0
        if icounter== self._ncounters and int((evt.xdata-1.0e-6)/1.5)==self._ncounters-1:
            icounter = self._ncounters-1
        if not (0 <= icounter < self._ncounters):
            print("HistCounts plot WARNING: invalid icounter encountered (should not happen)")
            if self._rect and self._rect.get_visible():
                self._rect.set_visible(False)
                evt.canvas.draw()
        self._update_rect_location(evt,icounter)

#FIXME: A lot of common code with _hoverhandler1d!!
class _hoverhandler2d:

    def __init__(self,figure,axes,hist2d):
        figure.canvas.mpl_connect("motion_notify_event", self.on_move)
        self._fig=figure
        self._h=hist2d
        self._axes=axes
        self._rect=None
        self._text=None
        self._text_prev_xfig=None
        self._text_prev_yfig=None
        self._rect_bin=None

    def _hovertext(self,ix,iy):
        tx=u'xbin    : %i (%g \u2264 x %s %g)'%(ix,self._h.getBinLowerX(ix),u'\u2264' if ix+1==self._h.nbinsx else '<',self._h.getBinUpperX(ix))
        ty=u'ybin    : %i (%g \u2264 y %s %g)'%(iy,self._h.getBinLowerY(iy),u'\u2264' if iy+1==self._h.nbinsy else '<',self._h.getBinUpperY(iy))
        return u'%s\n%s\ncontent: %g'%(tx,ty,self._h.getBinContent(ix,iy))

    def _update_rect_location(self,evt,ix,iy):
        if not self._rect:
            self._rect_bin=(ix,iy)
            self._rect=mplp.Rectangle((self._h.getBinLowerX(ix),self._h.getBinLowerY(iy)), self._h.binwidthx,self._h.binwidthy, alpha=0.1,color='b')
            self._text = evt.inaxes.text(evt.xdata,evt.ydata,self._hovertext(ix,iy),
                                         verticalalignment='bottom',horizontalalignment='left',multialignment='left',
                                         bbox={'facecolor':(0.8,0.8,1.0),'alpha':0.5,'edgecolor':'black', 'boxstyle':'round,pad=0.5'},
                                         family='monospace')
            _resize_text_reltofig(self._fig,self._text,0.4)
            evt.inaxes.add_patch(self._rect)
            _hovertext_setpos(self,evt)
            evt.canvas.draw()
            return
        assert self._rect and self._text and self._rect_bin is not None
        _hovertext_setpos(self,evt)
        if self._rect_bin==(ix,iy):
            if not self._rect.get_visible():
                self._rect.set_visible(True)
                self._text.set_visible(True)
            evt.canvas.draw()
            return
        self._rect.set_xy((self._h.getBinLowerX(ix),self._h.getBinLowerY(iy)))
        self._rect_bin=(ix,iy)
        self._text.set_text(self._hovertext(ix,iy))
        evt.canvas.draw()

    def on_move(self,evt):
        if not evt or self._axes!=evt.inaxes:
            if self._rect and self._rect.get_visible():
                self._rect.set_visible(False)
                self._text.set_visible(False)
                evt.canvas.draw()
            return
        ix=self._h.valueToBinX(evt.xdata)
        iy=self._h.valueToBinY(evt.ydata)
        ix=_fixbin(ix,evt.xdata,self._h.xmin,self._h.xmax,self._h.binwidthx,self._h.nbinsx)
        iy=_fixbin(iy,evt.ydata,self._h.ymin,self._h.ymax,self._h.binwidthy,self._h.nbinsy)
        if not ((0 <= ix < self._h.nbinsx) and (0 <= iy < self._h.nbinsy)):
            print("Hist2D plot WARNING: invalid ibin encountered (should not happen)")
            if self._rect and self._rect.get_visible():
                self._rect.set_visible(False)
                evt.canvas.draw()
        self._update_rect_location(evt,ix,iy)

class _draghandler:
    """generic draghandler - should work with artists with picking enabled"""
    def __init__(self, figure,axes) :
        self.dragged = None
        self.axes = axes
        figure.canvas.mpl_connect("pick_event", self.on_pick_event)
        figure.canvas.mpl_connect("button_release_event", self.on_release_event)
        figure.canvas.mpl_connect("motion_notify_event", self.on_motion)
        import matplotlib
        self._type_legend=matplotlib.legend.Legend
    def on_pick_event(self, event):
        if event.artist:
            if isinstance(event.artist,self._type_legend):
                #legends should be instead made draggable with leg.draggable(True) (or leg.set_draggable(True) in newer releases)
                return
            if not hasattr(event.artist,'get_position'):
                return
            self.dragged = event.artist
            self.pick_pos = (event.mouseevent.x,event.mouseevent.y)
            self.old_pos = self.dragged.get_position()
            return True
    def on_motion(self, evt):
        if self.dragged and evt.canvas:
            delta = (evt.x - self.pick_pos[0],evt.y - self.pick_pos[1])
            np=self.axes.transAxes.transform_point(self.old_pos)+delta
            self.dragged.set_position(self.axes.transAxes.inverted().transform_point(np))
            evt.canvas.draw()
    def on_release_event(self, event):
        self.dragged = None

class _keyhandler:
    def __init__(self,thefig,statbox,infobox,custom=None):
        self.infobox = infobox
        self.statbox = statbox
        self.statbox_savedpos = None
        self.fig=thefig
        self.cid = thefig.canvas.mpl_connect('key_press_event', self)
        self.custom=custom
    def __call__(self,evt):
        #cf. http://matplotlib.org/users/navigation_toolbar.html#navigation-keyboard-shortcuts
        #we add q/quit (above), b/statbox, F1/ctrl-h/info (todo)
        if not evt.name=='key_press_event' or not evt.key:
            return
        key=evt.key.lower()
        needdraw=True
        if key in ['i','ctrl+h','f1','escape']:
            self.infobox.set_visible(not self.infobox.get_visible())
        elif key=='b':
            sbp=self.statbox.get_position()
            in_corner = (abs(sbp[0]-self.statbox.statbox_corner_pos[0])<1.0e-9 and abs(sbp[1]-self.statbox.statbox_corner_pos[1])<1.0e-9)
            if in_corner and not self.statbox.get_visible():
                if self.statbox_savedpos is not None:
                    self.statbox.set_position(self.statbox_savedpos)
                self.statbox.set_visible(True)
            elif in_corner and self.statbox.get_visible():
                self.statbox.set_visible(False)
            else:
                self.statbox_savedpos = sbp
                self.statbox.set_position(self.statbox.statbox_corner_pos)
        elif self.custom and evt.key in self.custom:
            if self.custom[evt.key](evt) == 'nodraw':
                needdraw = False
        else:
            return False
        #we did something, so block the key for other handlers + trigger canvas update.
        evt.key=None#trick to block
        if needdraw:
            evt.canvas.draw()
        return True

def _add_quit_hook():
    if 'q' not in plt.rcParams['keymap.quit']:
        plt.rcParams['keymap.quit']=tuple(list(plt.rcParams['keymap.quit'])+['q','Q'])

def _set_window_title(hist,fig,extra=''):
    t='Simple%s%s'%(hist.__class__.__name__,extra)
    if hist.title:
        t+=' '+hist.title
    #NB: This also becomes the default for savefig (perhaps...):
    try:
        fig.canvas.set_window_title(t)
    except AttributeError:
        pass

#The following colormap was created by:
#cols=[]
#for x in [0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9,1.0]:
#    cols+=[plt.cm.spectral(x)]
#It is hardcoded here to guard against surprises (perhaps a global normalise
#call would screw it up, or it would look different in different matplotlib versions)
_colmap1d=[(0.53068823529411768, 0.0, 0.59738431372549017, 1.0),
           (0.0, 0.0, 0.86670000000000003, 1.0),
           (0.0, 0.59477254901960785, 0.86670000000000003, 1.0),
           (0.0, 0.66669999999999996, 0.5333, 1.0),#a good default
           (0.0, 0.73853137254901957, 0.0, 1.0),
           (0.0, 1.0, 0.0, 1.0),
           (0.93591568627450983, 0.9280725490196079, 0.0, 1.0),
           (1.0, 0.59999999999999998, 0.0, 1.0),
           (0.86408431372549022, 0.0, 0.0, 1.0),
           (0.80000000000000004, 0.80000000000000004, 0.80000000000000004, 1.0)]

def statsText(hist1d):
    stats=[('integral','%g'%hist1d.integral)]
    if not hist1d.empty():
        stats+=[('mean','%g'%hist1d.mean)]
        stats+=[('rms','%g'%hist1d.rms)]
        medianbin = hist1d.getPercentileBin(0.5)
        assert -1<=medianbin<=hist1d.nbins
        if medianbin==-1:
            mediantext = 'underflow'
        elif medianbin==hist1d.nbins:
            mediantext = 'overflow'
        else:
            mediantext = u'%g \u00b1 %g'%(hist1d.getBinCenter(medianbin),0.5*hist1d.getBinWidth())
        stats+=[('median',mediantext)]
        stats+=[('min','%g'%hist1d.minfilled)]
        stats+=[('max','%g'%hist1d.maxfilled)]
    if hist1d.underflow:
        stats+=[('underflow','%g'%hist1d.underflow)]
    if hist1d.overflow:
        stats+=[('overflow','%g'%hist1d.overflow)]
    return stats

def formatStatsText(stats):
    statwidth0 = max(len(s[0]) for s in stats)
    # statwidth1 = max(len(s[1]) for s in stats)
    return '\n'.join('%s = %s'%(s[0].ljust(statwidth0),s[1]) for s in stats)

def plot1d(hist,show=True,statbox=True,statbox_exactcorner=False,figure=None,axes=None,skip_backend_check=False):
    if not skip_backend_check:
        _ensure_backend_ok()
    if not figure or not axes:
        assert not figure and not axes,"either supply none or both of figure and axes"
        fig,ax = plt.subplots()
        fig.subplots_adjust(right=0.94,top=0.92)
        fig.patch.set_facecolor('w')#for .show(), not .savefig()
    else:
        fig,ax=figure,axes

    bars=ax.bar(**hist.bar_args())
    error_markers, ecaplines, ebarlinecols=ax.errorbar(**hist.errorbar_args())
    if hist.maxcontent<=0:
        ax.set_ylim(0.0,1.0)
    else:
        ax.set_ylim(0.0)#always start y-axis from 0
    ax.set_xlim(hist.xmin,hist.xmax)
    ax.set_title(hist.title)#not making dragable for estetics reasons
    ax.set_xlabel(hist.xlabel,picker=True)#todo: should snap to center/right ...
    ax.set_ylabel(hist.ylabel,picker=True)#todo: ... or have a shortcut key
    ax.grid()

    stats = statsText(hist)
    sb = _add_statbox(fig,ax,stats,vis=statbox,snap_to_corner=statbox_exactcorner)
    ib = _add_help_text(fig,ax,hist.histType())

    #mode, colors and linewidths:
    modes=['b','e','be',]#b:bars, e:errors
    errorstyles=['1','2','1o','2o']#lw followed by: .:thin markers, o:thick markers

    linewidths=[0,1,2]
    #state-tracking: use list of int rather than int to ensure sharing
    icol=[3]#
    ilw=[0]
    imode=[0]
    iestyle=[0]
    def update_collwmode():
        ls,ecol='solid','black'#default outlines are solid and black
        col=_colmap1d[icol[0]]
        lw=linewidths[ilw[0]]
        mode=modes[imode[0]]
        estyle=errorstyles[iestyle[0]]
        #1) bars:
        if 'b' in mode:
            if not lw and col=='w':
                lw,ls=1,'dotted'#prevent invisible
            if not lw:
                #for precision we treat lw=0 as lw=1 width edgecol=facecol
                lw,ecol=1,col
            for p in bars.patches:
                p.set_visible(True)
                p.set_color(col)#a bit redundant
                p.set_facecolor(col)
                p.set_edgecolor(ecol)
                p.set_linewidth(lw)
                p.set_linestyle(ls)
        else:
            for p in bars.patches:
                p.set_visible(False)
        error_lines = ecaplines+ebarlinecols
        if 'e' in mode:
            for p in error_lines+(error_markers,):
                p.set_visible(True)
            #markers:
            if '.' in estyle:
                error_markers.set_marker('.')
            elif 'o' in estyle:
                error_markers.set_marker('o')
            else:
                error_markers.set_visible(False)
            elw=int(estyle[0])
            for p in error_lines:
                p.set_linewidth(elw)
            for p in ecaplines:
                p.set_ms(elw*4)
                p.set_mew(elw)
        else:
            for p in error_lines+(error_markers,):
                p.set_visible(False)

    update_collwmode()#init

    _set_window_title(hist,fig)

    if not show:
        return fig,ax

    def toggle_col_next(evt):
        if 'b' not in modes[imode[0]]:
            imode[0]=modes.index('be')#enable bars
        else:
            icol[0] = (icol[0]+1)%len(_colmap1d)
        update_collwmode()
    def toggle_col_back(evt):
        if 'b' not in modes[imode[0]]:
            imode[0]=modes.index('be')#enable bars
        else:
            icol[0] = (icol[0]-1)%len(_colmap1d)
        update_collwmode()
    def toggle_edge(evt):
        ilw[0] = (ilw[0]+1)%len(linewidths)
        update_collwmode()
    def toggle_mode(evt):
        imode[0] = (imode[0]+1)%len(modes)
        update_collwmode()
    def toggle_errorstyle(evt):
        if 'e' not in modes[imode[0]]:
            imode[0]=modes.index('be')#enable errors
        else:
            iestyle[0] = (iestyle[0]+1)%len(errorstyles)
        update_collwmode()

    def do_clone(evt):
        hclone=hist.clone()
        plot1d(hclone,statbox=statbox,statbox_exactcorner=statbox_exactcorner)
        return 'nodraw'

    def do_norm_insideonly(evt):
        hclone=hist.clone()
        hclone.norm()
        plot1d(hclone,statbox=statbox,statbox_exactcorner=statbox_exactcorner)
        return 'nodraw'

    def do_norm(evt):
        hclone=hist.clone()
        hclone.norm()
        plot1d(hclone,statbox=statbox,statbox_exactcorner=statbox_exactcorner)
        return 'nodraw'

    def do_rebin(evt):
        new_nbins=None
        i,n = 2,hist.nbins
        while i<n:
            if n%i==0:
                new_nbins=n//i
                break
            i+=1
        if new_nbins is None:
            return#no rebinning possible
        hclone=hist.clone()
        hclone.rebin(new_nbins)
        plot1d(hclone,statbox=statbox,statbox_exactcorner=statbox_exactcorner)
        return 'nodraw'


    _keepalive = []
    if _interactive:
        dragh = _draghandler(fig,ax)
        keyh = _keyhandler(fig,sb,ib,custom={'m':toggle_col_next,'n':toggle_col_back,
                                           '1':do_norm,'ctrl+1':do_norm_insideonly,'u':do_clone,
                                           't':toggle_edge,'a':toggle_mode,'e':toggle_errorstyle,
                                           '2':do_rebin})
        hh=_hoverhandler1d(fig,ax,hist)
        _keepalive += [dragh,keyh,hh]
        _add_quit_hook()
    if show!='almost':
        plt.show()
    del _keepalive # todo: return as well?
    return fig,ax

def plotcounts(hist,show=True,statbox=False,statbox_exactcorner=False,figure=None,axes=None):
    _ensure_backend_ok()
    if not figure or not axes:
        assert not figure and not axes,"either supply none or both of figure and axes"
        fig,ax = plt.subplots()
        fig.subplots_adjust(right=0.94,top=0.92)
        fig.patch.set_facecolor('w')#for .show(), not .savefig()
    else:
        fig,ax=figure,axes

    counters=hist.counters
    width=1.0
    spacing=0.5
    bars=ax.bar(**hist.bar_args(width=width,spacing=spacing))
    error_markers, ecaplines, ebarlinecols=ax.errorbar(**hist.errorbar_args(width=width,spacing=spacing))

    if hist.maxcontent<=0:
        ax.set_ylim(0.0,1.0)
    else:
        ax.set_ylim(0.0)#always start y-axis from 0 (FIXME not for log)

    ax.set_xlim(-spacing,len(counters)*(spacing+width))
    ax.set_title(hist.title)#not making dragable for estetics reasons
    ax.set_xlabel(hist.xlabel,picker=True)#todo: should snap to center/right ...
    ax.set_ylabel(hist.ylabel,picker=True)#todo: ... or have a shortcut key
    ax.yaxis.grid(True,color='white',linestyle='-') #horizontal lines
    ax.xaxis.set_ticks_position('none')
    ax.yaxis.set_ticks_position('left')

    #FIXME: After zooming, labels are in the wrong place!! Also, should have shortcut for toggling labels above/ataxis


    #adhoc attempt at expanding y-axis a bit if there is a chance that counter
    #display labels will clash with title:
    ymin, ymax = ax.get_ylim()
    if hist.maxcontent>0 and ymax>0 and hist.maxcontent/ymax > 0.95:
        ax.set_ylim(0.0,ymax*1.05)

    if True:
        ax.set_xticks([])
        labels=[]
        for i,c in enumerate(counters):
            center=i*(width+spacing)+0.5*width
            x,y=ax.transAxes.inverted().transform_point(ax.transData.transform_point((center,c.value)))
            ll=ax.text(x,y+0.02,c.displaylabel,transform=ax.transAxes,size='small',
                      verticalalignment='bottom',horizontalalignment='center',multialignment='left',picker=True)
            labels+=[ll]
            #bbox={'facecolor':(0.8,0.8,1.0),'alpha':1.0,'edgecolor':'black', 'boxstyle':'round,pad=1'},
            #         family='monospace',picker=True)
    else:
        ax.set_xticks([i*(width+spacing)+0.5*width for i in range(len(counters))])
        ax.set_xticklabels([c.displaylabel for c in counters],rotation=0, size='small')

    ax.spines['top'].set_visible(False)
    ax.spines['right'].set_visible(False)

    stats=[('integral','%g'%hist.integral)]
    sb = _add_statbox(fig,ax,stats,vis=statbox,snap_to_corner=statbox_exactcorner)
    ib = _add_help_text(fig,ax,hist.histType())

    #mode, colors and linewidths:
    modes=['b','e','be',]#b:bars, e:errors
    errorstyles=['1','2','1o','2o']#lw followed by: .:thin markers, o:thick markers

    linewidths=[0,1,2]
    #state-tracking: use list of int rather than int to ensure sharing
    icol=[3]#
    ilw=[0]
    imode=[0]
    iestyle=[0]
    def update_collwmode():
        ls,ecol='solid','black'#default outlines are solid and black
        col=_colmap1d[icol[0]]
        lw=linewidths[ilw[0]]
        mode=modes[imode[0]]
        estyle=errorstyles[iestyle[0]]
        #1) bars:
        if 'b' in mode:
            if not lw and col=='w':
                lw,ls=1,'dotted'#prevent invisible
            if not lw:
                #for precision we treat lw=0 as lw=1 width edgecol=facecol
                lw,ecol=1,col
            for p in bars.patches:
                p.set_visible(True)
                p.set_color(col)#a bit redundant
                p.set_facecolor(col)
                p.set_edgecolor(ecol)
                p.set_linewidth(lw)
                p.set_linestyle(ls)
        else:
            for p in bars.patches:
                p.set_visible(False)
        error_lines = ecaplines+ebarlinecols
        if 'e' in mode:
            for p in error_lines+(error_markers,):
                p.set_visible(True)
            #markers:
            if '.' in estyle:
                error_markers.set_marker('.')
            elif 'o' in estyle:
                error_markers.set_marker('o')
            else:
                error_markers.set_visible(False)
            elw=int(estyle[0])
            for p in error_lines:
                p.set_linewidth(elw)
            for p in ecaplines:
                p.set_ms(elw*4)
                p.set_mew(elw)
        else:
            for p in error_lines+(error_markers,):
                p.set_visible(False)

    update_collwmode()#init

    _set_window_title(hist,fig)

    if not show:
        return fig,ax

    def toggle_col_next(evt):
        if 'b' not in modes[imode[0]]:
            imode[0]=modes.index('be')#enable bars
        else:
            icol[0] = (icol[0]+1)%len(_colmap1d)
        update_collwmode()
    def toggle_col_back(evt):
        if 'b' not in modes[imode[0]]:
            imode[0]=modes.index('be')#enable bars
        else:
            icol[0] = (icol[0]-1)%len(_colmap1d)
        update_collwmode()
    def toggle_edge(evt):
        ilw[0] = (ilw[0]+1)%len(linewidths)
        update_collwmode()
    def toggle_mode(evt):
        imode[0] = (imode[0]+1)%len(modes)
        update_collwmode()
    def toggle_errorstyle(evt):
        if 'e' not in modes[imode[0]]:
            imode[0]=modes.index('be')#enable errors
        else:
            iestyle[0] = (iestyle[0]+1)%len(errorstyles)
        update_collwmode()

    def do_clone(evt):
        hclone=hist.clone()
        plotcounts(hclone,statbox=statbox,statbox_exactcorner=statbox_exactcorner)
        return 'nodraw'

    def do_norm_insideonly(evt):
        hclone=hist.clone()
        hclone.norm()
        plotcounts(hclone,statbox=statbox,statbox_exactcorner=statbox_exactcorner)
        return 'nodraw'

    def do_norm(evt):
        hclone=hist.clone()
        hclone.norm()
        plotcounts(hclone,statbox=statbox,statbox_exactcorner=statbox_exactcorner)
        return 'nodraw'

    _keepalive = []
    if _interactive:
        dragh=_draghandler(fig,ax)
        keyh=_keyhandler(fig,sb,ib,custom={'m':toggle_col_next,'n':toggle_col_back,
                                           '1':do_norm,'ctrl+1':do_norm_insideonly,'u':do_clone,
                                           't':toggle_edge,'a':toggle_mode,'e':toggle_errorstyle,
                                           'k':lambda x:None,'L':lambda x:None#Block xaxis log-scale switching (TODO: 'L')
                                       })
        hh=_hoverhandlercounts(fig,ax,hist)
        _keepalive += [dragh,keyh,hh]
    _add_quit_hook()
    if show!='almost':
        plt.show()
    del _keepalive # todo: return as well?
    return fig,ax

def _mpl_get_cmap( arg ):
    if hasattr(matplotlib,'colormaps'):
        if hasattr(matplotlib.colormaps,'get_cmap'):
            #Recommended since matplotlib 3.7:
            return matplotlib.colormaps.get_cmap( arg )
    #Deprecated in matplotlib 3.7:
    return matplotlib.cm.get_cmap(arg)

def _has_cmap(cm):
    if cm in plt.cm.datad:
        return True
    try:
        _mpl_get_cmap(cm)
    except ( AttributeError, ValueError ):
        return False
    return True

def _mpl_register_cmap(name, cmap, fail_if_present = True ):
    if fail_if_present and _has_cmap(name):
        return
    if hasattr(matplotlib,'colormaps'):
        #force to allow overwrite:
        matplotlib.colormaps.register(name=name,cmap=cmap,force=True)
    else:
        #deprecated? at least it stopped working.
        plt.register_cmap(name=name,cname=cname)

def plot2d_lego(hist,show=True,cmap=None):
    _ensure_backend_ok()
    if not cmap:
        cmap = 'plasma' if _has_cmap('plasma') else 'spectral'
    from mpl_toolkits.mplot3d import Axes3D # noqa F401
    fig = plt.figure()
    ax = fig.add_subplot(111, projection='3d')
    fig.subplots_adjust(left=0.03,right=0.97,bottom=0.07,top=0.98)
    fig.patch.set_facecolor('w')#for .show(), not .savefig()
    ax.bar3d(zsort='average',**hist.bar3d_args(cmap=cmap))
    ax.set_title(hist.title)
    ax.set_xlabel(hist.xlabel)
    ax.set_ylabel(hist.ylabel)
    _set_window_title(hist,fig,' LEGO')
    if show:
        _add_quit_hook()
        if show!='almost':
            plt.show()
    return fig,ax

def plot2d(hist,show=True,cmap=None,statbox=False,statbox_exactcorner=False,figure=None,axes=None,aspect='auto',
           extra_imshow_args={},extra_colorbar_args={}):
    _ensure_backend_ok()
    from mpl_toolkits.axes_grid1 import make_axes_locatable

    if not figure or not axes:
        assert not figure and not axes,"either supply none or both of figure and axes"
        fig,ax = plt.subplots()
        fig.subplots_adjust(right=0.94,top=0.92)
        fig.patch.set_facecolor('w')#for .show(), not .savefig()
    else:
        fig,ax=figure,axes

    tmp=dict(hist.imshow_args().items())

    #For consistency across matplotlib versions, pick a reasonable(?) list of colourmaps:
    cmaps = [ #perceptual:
              'plasma','viridis', 'inferno',
              #sequential
              'binary', 'pink','afmhot','YlOrRd','PuRd','PuBu',
              #diverging:
              'coolwarm',
              #misc:
              'terrain', 'gist_stern', 'nipy_spectral']

    #Modify proposed cmaps depending on availablity:
    cmaps_ok = [ cm for cm in cmaps if _has_cmap(cm) ]
    if not (cmaps_ok and cmaps_ok[0]=='plasma') and _has_cmap('spectral'):
        cmaps_ok = ['spectral'] + cmaps_ok#reproduce old default behaviour for older matplotlib
    if not cmaps_ok:
        print("WARNING: Could not find any of the usual cmaps")
        cmaps_ok = list(sorted(plt.cm.datad.keys()))
    cmaps = cmaps_ok

    if cmap is None:
        cmap = cmaps[0]

    try:
        from PyAnaMisc.dyncmap import dynamic_2d_colormap
        _,_,dynamic_cmap=dynamic_2d_colormap(tmp['X'])
        _mpl_register_cmap(name=dynamic_cmap.name, cmap=dynamic_cmap)
        #to not break other functionality, make a fake copy for the "reverse" of
        #this custom map as well:
        import copy
        dynamic_cmap_r = copy.deepcopy(dynamic_cmap)
        dynamic_cmap_r.name = dynamic_cmap.name+'_r'
        _mpl_register_cmap(name=dynamic_cmap_r.name, cmap=dynamic_cmap_r)#register a fake inverse
        cmaps+=[dynamic_cmap.name]
    except ImportError:
        pass
    if cmap not in cmaps:
        #query matplotlib for cmap (will throw ValueError if not present - we let it propagate to the caller)
        _mpl_get_cmap(cmap)
        #no exceptions thrown in previous call - add this at front of list:
        cmaps = [cmap] + cmaps
    assert cmap in cmaps
    interpolation='nearest'

    tmp.update(extra_imshow_args)
    im = plt.imshow(interpolation=interpolation,cmap=cmap,aspect=aspect,**tmp)
    ax.set_title(hist.title)#not making dragable (center seems to be always the right choice)
    ax.set_xlabel(hist.xlabel,picker=True)#todo: should snap to center/right ...
    ax.set_ylabel(hist.ylabel,picker=True)#todo: ... or have a shortcut key

    divider = make_axes_locatable(ax)
    cax = divider.append_axes("right", size="5%", pad=0.05)
    if hist.empty():
        im.set_clim(0.0,1.0)

    plt.colorbar(im, cax=cax, **extra_colorbar_args)#,ticks=[])

    ig = hist.integral
    stats=[('integral','%g'%ig)]
    if not hist.empty():
        stats+=[('meanx','%g'%hist.meanx)]
        stats+=[('rmsx','%g'%hist.rmsx)]
        stats+=[('meany','%g'%hist.meany)]
        stats+=[('rmsy','%g'%hist.rmsy)]
        stats+=[('covxy','%g'%hist.covxy)]
        stats+=[('corxy','%g'%hist.corxy)]
        stats+=[('xmin','%g'%hist.minfilledx)]
        stats+=[('xmax','%g'%hist.maxfilledx)]
        stats+=[('ymin','%g'%hist.minfilledy)]
        stats+=[('ymax','%g'%hist.maxfilledy)]
    if hist.underflowx:
        stats+=[('underflowx','%g'%hist.underflowx)]
    if hist.overflowx:
        stats+=[('overflowx','%g'%hist.overflowx)]
    if hist.underflowy:
        stats+=[('underflowy','%g'%hist.underflowy)]
    if hist.overflowy:
        stats+=[('overflowy','%g'%hist.overflowy)]
    sb = _add_statbox(fig,ax,stats,vis=statbox,snap_to_corner=statbox_exactcorner)
    ib = _add_help_text(fig,ax,int(hist.histType()))

    _set_window_title(hist,fig)

    if not show:
        return fig,ax

    _keepalive = []
    if _interactive:
        dragh=_draghandler(fig,ax)
        _keepalive.append(dragh)
    def toggle_cmap(evt):
        n,ext=im.get_cmap().name,''
        if n.endswith('_r'):
            n,ext=n[:-2],'_r'
        im.set_cmap(cmaps[(cmaps.index(n)+1)%len(cmaps)]+ext)
    def toggle_cmap_back(evt):
        n,ext=im.get_cmap().name,''
        if n.endswith('_r'):
            n,ext=n[:-2],'_r'
        im.set_cmap(cmaps[(len(cmaps)+cmaps.index(n)-1)%len(cmaps)]+ext)
    def toggle_cmap_reverse(evt):
        n=im.get_cmap().name
        n=n[:-2] if n.endswith('_r') else n+'_r'
        im.set_cmap(n)
    def toggle_im_interpolation(evt):
        opts=['nearest','bilinear','bicubic']
        o=opts[(opts.index(im.get_interpolation())+1)%len(opts)]
        im.set_interpolation(o)

    def do_lego(evt):
        plot2d_lego(hist,cmap=im.get_cmap().name)
        return 'nodraw'

    def do_clone(evt):
        hclone=hist.clone()
        plot2d(hclone,statbox=statbox,statbox_exactcorner=statbox_exactcorner,cmap=im.get_cmap().name)
        return 'nodraw'

    def do_norm_insideonly(evt):
        hclone=hist.clone()
        hclone.norm()
        plot2d(hclone,statbox=statbox,statbox_exactcorner=statbox_exactcorner,cmap=im.get_cmap().name)
        return 'nodraw'

    def do_norm(evt):
        hclone=hist.clone()
        hclone.norm()
        plot2d(hclone,statbox=statbox,statbox_exactcorner=statbox_exactcorner,cmap=im.get_cmap().name)
        return 'nodraw'

    if _interactive:
        keyh=_keyhandler(fig,sb,ib,custom={'m':toggle_cmap,'n':toggle_cmap_back,'t':toggle_cmap_reverse,
                                           'a':toggle_im_interpolation,'d':do_lego,
                                           '1':do_norm,'ctrl+1':do_norm_insideonly,'u':do_clone,
                                           'k':lambda x:None,'l':lambda x:None#Block log-scale switching in 2D plots
                                       })
        hh=_hoverhandler2d(fig,ax,hist)
        _keepalive += [ keyh, hh ]
    _add_quit_hook()
    if show!='almost':
        plt.show()
    del _keepalive # todo: return as well?
    return fig,ax

def _fill_between(x, y1, y2=0, ax=None, **kwargs):
    """Plot filled region between `y1` and `y2`.

    This function works exactly the same as matplotlib's fill_between, except
    that it also plots a proxy artist (specifically, a rectangle of 0 size)
    so that it can be added it appears on a legend.
    """
    ax = ax if ax is not None else plt.gca()
    ax.fill_between(x, y1, y2, **kwargs)
    p = plt.Rectangle((0, 0), 0, 0, **kwargs)
    ax.add_patch(p)
    return p

try:
    _defoverlaycolmap_basecm = plt.cm.nipy_spectral
except AttributeError:
    _defoverlaycolmap_basecm = plt.cm.spectral

def _defoverlaycolmap(x):
    a,b=0.15,0.9
    return _defoverlaycolmap_basecm(a + x * (b-a))

def overlay(hists,labels,colors=None,
            title=None,xlabel=None,ylabel=None,
            figure=None,axes=None,show=True,
            errors=1.0,
            xmin=None,xmax=None,ymin=None,ymax=None,
            logx=False,logy=False,
            legend=True,legend_title=None,cmap=None,
            clip_for_log=False,plotargs=None):
    """Take a list of Hist1D instances and an associated list of legend labels and produce a comparison plot.
    Annotations will be taken from the first histogram if not specified.
    Due to a matplotlib bug it might be necessary to set clip_for_log=True when doing log plots"""
    assert len(hists)>=1#1 is a bit weird, but no need to crash automatic scripts
    assert len(hists)==len(labels)
    assert colors is None or len(hists)==len(colors)
    #fixme: keymap for logy should make sure to enable the clipping!
    _ensure_backend_ok()

    if not cmap:
        cmap=_defoverlaycolmap
    if title is None:
        title = hists[0].title
    if xlabel is None:
        xlabel = hists[0].xlabel
    if ylabel is None:
        ylabel = hists[0].ylabel

    if not figure or not axes:
        assert not figure and not axes,"either supply none or both of figure and axes"
        fig,ax = plt.subplots()
        fig.subplots_adjust(right=0.94,top=0.92)
        fig.patch.set_facecolor('w')#for .show(), not .savefig()
    else:
        fig,ax=figure,axes

    if title is not False:
        ax.set_title(title)#not making dragable (center seems to be always the right choice)
    if xlabel is not False:
        ax.set_xlabel(xlabel,picker=True)#todo: should snap to center/right ...
    if ylabel is not False:
        ax.set_ylabel(ylabel,picker=True)#todo: ... or have a shortcut key

    lines=[]
    proxy_artists=[]
    import copy
    for i,(hist,legkey) in enumerate(zip(hists,labels)):
        col=colors[i] if (colors is not None) else _col(cmap,i,len(hists))
        extra_args = {} if not plotargs else copy.deepcopy(plotargs[i])
        alpha_errors = 0.2
        if 'alpha' in extra_args and isinstance(extra_args['alpha'],float):
            alpha_errors *= extra_args['alpha']
        zorder_errors = None
        if 'zorder' in extra_args and (isinstance(extra_args['zorder'],float) or isinstance(extra_args['zorder'],int)):
            zorder_errors = extra_args['zorder']-0.01
        if 'color' not in extra_args:
            extra_args['color']=col
        else:
            col = extra_args['color']
        x,y=hist.curve()
        lines+=[ax.plot(x,y,label=legkey,**extra_args)]#,picker=False)]#picker as integer means pick radius
        if errors:
            x,errlow,errhigh=hist.errorband(ndev=errors,clip_for_log=clip_for_log)
            p=_fill_between(x,errlow,errhigh, ax=ax, color=col,alpha=alpha_errors,zorder=zorder_errors)
            proxy_artists+=[(p,legkey)]

    #ax.set_xlim(0.0,2000.0)

    if logy:
        ax.set_yscale('log',nonposy='clip')
    else:
        if ymin is None:
            ymin=0.0
    if logx:
        ax.set_xscale('log')

    ax.set_xlim(xmin=xmin,xmax=xmax)
    ax.set_ylim(ymin=ymin,ymax=ymax)

    if legend:
        if not errors:
            leg=ax.legend(title=legend_title)
            #Added here by KK and committed by mistake: plt.setp(leg.get_title(),fontsize='14')
        else:
            handles, labels = ax.get_legend_handles_labels()
            import matplotlib.legend_handler as lh
            my_handler = lh.HandlerLine2D(numpoints=1,marker_pad=0.)
            leg=ax.legend([(p,h) for (p,ll),h in zip(proxy_artists,handles)],
                          [ll for p,ll in proxy_artists],handler_map={plt.Line2D:my_handler},
                          title=legend_title)
            #Added here by KK and committed by mistake: plt.setp(leg.get_title(),fontsize='14')
        #if legend_title:
        #    leg.get_title().set_ha('left')
        #    leg.get_title().set_fontsize('small')
        if hasattr(leg,'set_draggable'):
            leg.set_draggable(True)
        else:
            leg.draggable(True)


    retval = (fig,ax,leg) if legend else (fig,ax)

    if not show:
        return retval
    _keepalive = []
    if _interactive:
        dragh=_draghandler(fig,ax)
        hh=_hoverhandleroverlay(fig,ax,lines,labels)
        _keepalive += [dragh,hh]
    _add_quit_hook()
    if show != 'almost':
        plt.show()
    del _keepalive # todo: return as well?
    return retval

def plot1d_overlay(self,hists,labels,colors=None,**args):
    """Convenience method which accepts N other hists and N+1 legend labels and plots via overlay([self]+hist,labels,**args)"""
    assert len(hists)+1==len(labels)
    return overlay([self]+hists,labels,colors=colors,**args)
