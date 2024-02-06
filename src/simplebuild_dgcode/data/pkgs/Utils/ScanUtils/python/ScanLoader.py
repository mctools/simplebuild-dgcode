
#for collecting the result of parameter scans. Look in the methods below to see
#how it is assumed the output is organised.

import os
import glob
import SimpleHists
import pathlib
class ScanJob:
    def __init__(self,jobdir,quiet_open=True,
                 grifffile='rundir*/runsample/*.griff',
                 histfile='rundir*/runana/*.shist'):
        jobdir = pathlib.Path(jobdir)
        self.__setup = None
        self.__jobdir=jobdir
        assert ( jobdir /'state.done' ).exists()
        assert ( jobdir /'exitcode.txt' ).exists()
        assert ( jobdir /'exitcode.txt' ).read_text().strip() == '0'
        sfs=glob.glob(os.path.join(jobdir,grifffile))
        assert len(sfs)==1
        self.__samplegrifffile = sfs[0]
        assert os.path.exists(self.__samplegrifffile)
        hfs=glob.glob(os.path.join(jobdir,histfile))
        assert len(hfs)==1
        self.__histfile=hfs[0]
        assert os.path.exists(self.__histfile)
        self.__hists=None
        self.__quiet_open = quiet_open
        cf=os.path.join(jobdir,'comment.txt')
        if os.path.exists(cf):
            fh=open(cf)
            self.__label = fh.read().strip()
            fh.close()
        else:
            print("WARNING: Missing label info (comment.txt) in %s"%os.path.basename(jobdir))
            self.__label='<badlabel>'

    def label(self):
        return self.__label

    def jobdir(self):
        return self.__jobdir

    def setup(self):
        if self.__setup is None:
            #important that we not keep a reference to the datareader (to not
            #have too many file handles open in large scans). Consequently we
            #must ref the setup instance to keep it around afterwards.
            from GriffDataRead import  GriffDataReader
            toggle_openmsg=False
            if self.__quiet_open and GriffDataReader.openMsg():
                toggle_openmsg=True
                GriffDataReader.setOpenMsg(False)
            dr=GriffDataReader(self.__samplegrifffile)
            if toggle_openmsg:
                GriffDataReader.setOpenMsg(True)
            self.__setup=dr.setup()
            self.__setup.ref()
            del dr
        return self.__setup

    def __getitem__(self,parname):
        #convenience method for access parameters flatly via []
        if not hasattr(self,'_flatpars'):
            from ScanUtils.JobExtract import flatten_pars
            self._flatpars = flatten_pars(self.setup())
        return self._flatpars[parname]

    def __del__(self):
        if self.__setup:
            self.__setup.unref()
            self.__setup=None

    def __histload(self):
        if self.__hists is None:
            self.__hists=SimpleHists.HistCollection(self.__histfile)
        return self.__hists

    def histcol(self):
        return self.__histload()

    def hists(self):
        return self.__histload().hist_getter

    def hist_names(self):
        return self.__histload().getKeys()

    def hist(self,histname):
        return self.__histload().hist(histname)

    def hist_file(self):
        return self.__histfile

def get_scan_jobs(scandirbase,quiet_open = True,dict_by_label=False):
    scandirbase=os.path.realpath(scandirbase)
    assert os.path.isdir(scandirbase)
    scandirs=glob.glob(os.path.join(scandirbase,'job*'))
    scandirs.sort()
    sj=[]
    for sdir in scandirs:
        if not os.path.exists(os.path.join(sdir,'state.done')):
            print("Ignoring unfinished job in %s/"%os.path.basename(sdir))
            continue
        sj+=[ScanJob(sdir,quiet_open=quiet_open)]
    if not dict_by_label:
        return sj
    d={}
    for j in sj:
        if j.label() in d:
            d[j.label()] += [j]
        else:
            d[j.label()] = [j]
    return d
