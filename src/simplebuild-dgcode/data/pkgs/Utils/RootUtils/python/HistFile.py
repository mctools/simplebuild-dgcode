import ROOT

def getHists(filename):
    import copy
    tfile=ROOT.TFile(filename)
    out={}
    for key in tfile.GetListOfKeys():
        o=key.ReadObj()
        if isinstance(o,ROOT.TH1):
            name=o.GetName()
            assert not name in out,"File %s has multiple histograms with name %s"%(filename,name)
            out[name]=copy.deepcopy(o)
    tfile.Close()
    del tfile
    return out

def writeObjectsToFile(filename,list_of_objects):
    f=ROOT.TFile(filename,"RECREATE")
    for o in list_of_objects:
        o.Write()
    f.Close()
    del f

def normalise_th1d_with_errors(h,percent=False):
    import math
    assert isinstance(h,ROOT.TH1D) or isinstance(h,ROOT.TH1F)
    n = h.GetEntries()#includes entries in over/underflow. Use h.Integral() to not include them.
    if not n:
        return
    scalefact = 1.0/n
    if percent:
        scalefact *= 100.0
    for ibin in range(h.GetNbinsX()+2):
        c = h.GetBinContent(ibin)
        h.SetBinContent(ibin,c*scalefact)
        h.SetBinError(ibin,math.sqrt(c)*scalefact)

def copyhist(hist,rebin=1,normalise=False,changename=False):
    import copy
    h=copy.deepcopy(hist)
    if changename:
        h.SetName("%s__copy__"%hist.GetName())
    if rebin!=1:
        h.Rebin(rebin)
    if normalise:
        normalise_th1d_with_errors(h,percent=True if normalise=='%' else False)
    return h

__pause_continue=False
def pause(back_button=False):
    global __pause_continue
    if __pause_continue:
        return
    r=input('press ENTER to continue [%sq=quit, a=all]\n'%('b=back, ' if back_button else ''))
    if back_button and r.lower() in ['b','back']:
        return 'back'
    if r.lower() in ['a','all']:
        __pause_continue=True
        return
    if r.lower() in ['q','quit']:
        raise SystemExit

def create_canvas(c=None):
    if not c:
        c=ROOT.TCanvas()
    c.ToggleEventStatus();
    c.ToggleToolTips();
    c.ToggleToolBar();
    c.ToggleEditor();
    c.SetGridx();
    c.SetGridy();
    c.SetTickx(1);
    c.SetTicky(1);
    return c

def points2tgraph(xvals,yvals=None):
    import ROOT
    n=len(xvals)
    assert yvals==None or n==len(yvals)
    vx=ROOT.TVectorD(n)
    vy=ROOT.TVectorD(n)
    if yvals==None:
        for i,(x,y) in enumerate(xvals):
            vx[i] = x
            vy[i] = y
    else:
        for i,x in enumerate(xvals):
            vx[i] = x
        for i,y in enumerate(yvals):
            vy[i] = y
    return ROOT.TGraph(vx,vy)

def make_graph(n,x,y,title="",titleX="",titleY="",markerColor=None,markerStyle=20,lineWidth=1,lineColor=None):
    if markerColor is None:
        markerColor = ROOT.kBlack
    g=ROOT.TGraph(n,x,y)
    g.SetTitle(title)
    g.GetXaxis().SetTitle(titleX)
    g.GetYaxis().SetTitle(titleY)
    g.SetMarkerColor(markerColor)
    g.SetMarkerStyle(markerStyle)
    g.SetLineColor(g.GetMarkerColor() if lineColor==None else lineColor)
    g.SetLineWidth(lineWidth)
    return g

def make_grapherrors(n,x,y,errX,errY,title="",titleX="",titleY="",markerColor=None,markerStyle=20,lineWidth=1,lineColor=None):
    if markerColor is None:
        markerColor = ROOT.kBlack
    g=ROOT.TGraphErrors(n,x,y,errX,errY)
    g.SetTitle(title)
    g.GetXaxis().SetTitle(titleX)
    g.GetYaxis().SetTitle(titleY)
    g.SetMarkerColor(markerColor)
    g.SetMarkerStyle(markerStyle)
    g.SetLineColor(g.GetMarkerColor() if lineColor==None else lineColor)
    g.SetLineWidth(lineWidth)
    return g


###########################
#### Plotting function ####
###########################

__ihist = 0
__html=[]
def cmphists(histograms_in,legend_header='',title=None,rebin=1,normalise='%',
             unitx=None,labelx=None,
             xmin=None,xmax=None,ymax=None,
             logx=None,logy=None,
             sort_by_legend=True,
             writeDir=None):
    #histograms_in is a list [(legendkey,histogram)]
    global __ihist
    global __html
    #title=''
    __ihist+=1
    ROOT.gStyle.SetOptStat(0)
    c=create_canvas(ROOT.TCanvas("c%i"%__ihist,title if title else 'plot %i'%__ihist))


    histograms = [(hlegendkey,copyhist(h,rebin=rebin,normalise=normalise)) for hlegendkey,h in histograms_in]
    if sort_by_legend:
        try:
            import natsort
            ns=natsort.natsorted
        except ImportError:
            print("WARNING: natsort module not found => using rudimentary sorting")
            ns=sorted
        histograms = ns(histograms)

    h0 = histograms[0][1]
    if title!=None: h0.SetTitle(title)
    else: title=h0.GetTitle()
    c.SetTitle(title)
    filename='plot_%03i_%s'%(__ihist,title)
    for badchar in ' /[]#().{}':
        filename=filename.replace(badchar,'_')

    #    #for h in histograms:
    #    nperevt_old = h_old_orig.GetEntries()/nevts_old
    #    nperevt_new = h_new_orig.GetEntries()/nevts_new
    #    print '======== cmphists: %s'%title
    #    if nperevt_old:
    #        reldiff='%f%%'%((nperevt_new-nperevt_old)*100.0/nperevt_old-1.0)
    #    else:
    #        if nperevt_new: reldiff='+inf'
    #        else: reldiff='undef'
    #    print 'n/evt_old = %f   n/evt_new = %f (difference %s)'%(nperevt_old,nperevt_new,reldiff)
    #

    colour_list = [ROOT.kBlue-9,ROOT.kGreen+2,ROOT.kOrange+5,ROOT.kGray+3,ROOT.kMagenta+2,ROOT.kOrange+10,ROOT.kBlue+3,ROOT.kRed]
    for i,(_,h) in enumerate(histograms):
        icol=i%len(colour_list)
        #h.SetFillColor(colour_list[i])
        h.SetLineColor(colour_list[icol])
        h.SetMarkerColor(colour_list[icol])
        h.SetLineWidth(2)
        #h.SetFillStyle(1)
        h.SetLineStyle(1)

    if unitx or labelx:
        h0.GetXaxis().SetTitle('%s [%s]'%(labelx,unitx) if labelx else unitx)

    if normalise:
        h0.GetYaxis().SetTitle('Fraction [%%] / %.2g%s'%(h0.GetBinWidth(2),' %s'%unitx if unitx else ''))

    if xmin!=None or xmax!=None:
        if xmin==None: xmin=h0.GetXaxis().GetXmin()
        if xmax==None: xmax=h0.GetXaxis().GetXmax()
        for _,h in histograms:
            h.GetXaxis().SetRangeUser(xmin,xmax)

    if ymax==None:
        ymax=max(h.GetMaximum() for _,h in histograms)
        ymax=ymax + abs(ymax)*0.1
    for _,h in histograms:
        h.SetMaximum(ymax)

    for i,(_,h) in enumerate(histograms):
        h.Draw('hist'+(' same' if i else ''))

    leg = ROOT.TLegend(0.7,0.75,0.9,0.9)
    if legend_header:
        leg.SetHeader(legend_header)
    for lk,h in histograms:
        leg.AddEntry(h," %s"%lk).SetOption("l")
    leg.SetFillColor(ROOT.kWhite)
    leg.SetMargin(0.18)
    leg.Draw()

    if logx:
        c.SetLogx();
    if logy:
        c.SetLogy();
    c.Update()
    c.Print('%s.png'%filename)
    c.Print('%s.pdf'%filename)
    c.Print('%s.svg'%filename)
    __html+=['<a href="%s.svg"><img src="%s.png"/></a>'%(filename,filename)]
    pause()
    if writeDir:
        cwd = ROOT.gDirectory
        writeDir.cd()
        c.Write()
        cwd.cd()
    del c

class HistMgr:
    import ROOT

    def __init__(self):
        self.__allhists=[]

    def book1d(self,*args):
        h=ROOT.TH1D(*args)
        self.__allhists+=[h]
        return h

    def book2d(self,*args):
        h=ROOT.TH2D(*args)
        self.__allhists+=[h]
        return h

    def write(self,filename):
        f=ROOT.TFile(filename,"RECREATE")
        for h in self.__all_hists:
            h.Write()
        f.Close()

def hist_decode_labelvals(h):
    #assume histogram has all bins with unique bin labels. Returns object with those bin values as properties
    class HistLabelVals:
        pass
    a=HistLabelVals()
    for ibin in range(1,h.GetNbinsX()+1):
        setattr(a,h.GetXaxis().GetBinLabel(ibin),float(h.GetBinContent(ibin)))
    return a
