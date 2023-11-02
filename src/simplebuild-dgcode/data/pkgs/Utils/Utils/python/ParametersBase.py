from Utils._ParametersBase import *

#add a few methods on the python side.

def _swallow_cmdline(self,args=None):
    """Swallows command line arguments of the form parname=value"""

    def _boolify_string(s):
        assert s!='' and s!=None#this is for strings with content
        if s.lower() in ['y','yes','1','true','on']: return True
        if s.lower() in ['n','no','0','false','off']: return False
        print('Invalid value for boolean:',s)
        assert False
        return False

    #make sure that we actually modify the list instance in question:
    if args==None:
        import sys
        arglist=sys.argv
        iarg=1
    else:
        arglist=args
        iarg=0

    #special case for forcepars variable (also, do not consume it!)
    for a in arglist:
        if not a.startswith('forcepars='): continue
        a_split = a.split('=')
        if len(a_split)==2 and _boolify_string(a_split[1]):
            self.setIgnoreRanges()

    while iarg<len(arglist):
        a_split = arglist[iarg].split('=',1)
        used=False
        if len(a_split)==2:
            name,val=a_split
            if name=='forcepars':
                iarg+=1
                continue
            used=True
            if self.hasParameterDouble(name): self.setParameterDouble(name,float(val))
            elif self.hasParameterInt(name): self.setParameterInt(name,int(val))
            elif self.hasParameterBoolean(name): self.setParameterBoolean(name,_boolify_string(val))
            elif self.hasParameterString(name): self.setParameterString(name,val)
            else:
                used=False
        if used:
            arglist.pop(iarg)
        else:
            iarg+=1

def _parambase_dir(self):
    pars=self.getParameterListDouble()+self.getParameterListInt()+self.getParameterListBoolean()+self.getParameterListString()
    return dir(self.__class__)+pars

def _parambase_getattr(self,name):
    if self.hasParameterDouble(name): return float(self.getParameterDouble(name))
    elif self.hasParameterInt(name): return int(self.getParameterInt(name))
    elif self.hasParameterBoolean(name): return bool(self.getParameterBoolean(name))
    elif self.hasParameterString(name): return str(self.getParameterString(name))
    else:
        raise AttributeError("%s has no '%s' attribute"%(self.__class__.__name__,name))

def _parambase_setattr(self,name,val):
    if self.hasParameterDouble(name): self.setParameterDouble(name,val)
    elif self.hasParameterInt(name): self.setParameterInt(name,val)
    elif self.hasParameterBoolean(name): self.setParameterBoolean(name,val)
    elif self.hasParameterString(name):
        if isinstance(val,list) or isinstance(val,set) or isinstance(val,tuple):
            val=';'.join(val)
        self.setParameterString(name,val)
    else:
        #allow clients to add whatever attributes they want for other purposes:
        super(ParametersBase,self).__setattr__(name,val)

def _getParameterList(self):
    l=self.getParameterListDouble()
    l+=self.getParameterListInt()
    l+=self.getParameterListBoolean()
    l+=self.getParameterListString()
    l.sort()
    return l

def _getParameterTypes(self):
    d={}
    d.update(dict((p,'dbl') for p in self.getParameterListDouble()))
    d.update(dict((p,'int') for p in self.getParameterListInt()))
    d.update(dict((p,'flg') for p in self.getParameterListBoolean()))
    d.update(dict((p,'str') for p in self.getParameterListString()))
    return d


def _asDict(self):
    d={}
    for p in self.getParameterListDouble():
        d[p] = self.getParameterDouble(p)
    for p in self.getParameterListInt():
        d[p] = self.getParameterInt(p)
    for p in self.getParameterListBoolean():
        d[p] = self.getParameterBoolean(p)
    for p in self.getParameterListString():
        d[p] = self.getParameterString(p)
    return d

ParametersBase.swallowCmdLine = _swallow_cmdline
ParametersBase.__dir__ = _parambase_dir
ParametersBase.__getattr__ = _parambase_getattr
ParametersBase.getParameter = _parambase_getattr
ParametersBase.__setattr__ = _parambase_setattr
ParametersBase.getParameterList = _getParameterList
ParametersBase.getParameterTypes = _getParameterTypes
ParametersBase.asDict = _asDict
