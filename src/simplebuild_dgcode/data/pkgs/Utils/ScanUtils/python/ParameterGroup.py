__metaclass__ = type#py2 backwards compatibility

#utility class which can be used to add a number of named parameters and their
#possible values and then generate all possible combinations of said parameters in a format suitable for the command line.

import itertools

class ParameterGroup:

    def __init__(self):
        self._pname2vals={}
        self._filters=[]

    def add(self,parname,parvalues,overwrite=False):
        if not overwrite and parname in self._pname2vals:
            raise Exception('Attempting to set parameter name twice: %s'%parname)
        if not hasattr(parvalues,'__iter__'):
            parvalues=[parvalues]
        self._pname2vals[parname]=parvalues

    def add_filter(self,f):
        self._filters+=[f]

    def products(self):
        l=[]
        for parname in sorted(self._pname2vals.keys()):
            l+=[[(parname,v) for v in self._pname2vals[parname]]]
        for p in itertools.product(*l):
            ok=True
            if self._filters:
                pars=dict(p)
                for f in self._filters:
                    if not f(pars):
                        ok=False
                        break
            if ok:
                yield p

    def cmd_line_args(self):
        #don't quote the arguments, the ScanLauncher will do that
        for p in self.products():
            l=[]
            for pn,pv in p:
                if isinstance(pv,str):
                    l+=['%s=%s'%(pn,pv)]
                elif isinstance(pv,bool):
                    l+=['%s=%s'%(pn,'true' if pv else 'false')]
                else:
                    l+=['%s=%g'%(pn,pv)]
            yield l
