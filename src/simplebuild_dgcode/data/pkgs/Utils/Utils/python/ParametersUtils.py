from __future__ import print_function

def summarise_params(parambase_list,ignore_same=False):
    #prints out differences and returns names of parameters which are same or different
    def parval(parambaseobject,parname):
        x=eval('parambaseobject.%s'%parname)
        return x
    par2valcount={}
    params=None
    for parambase in parambase_list:
        if params==None:
            #first time
            params=parambase.getParameterList()
            for p in params: par2valcount[p]={}
        else:
            #must be same parameters:
            assert params==parambase.getParameterList()
        for p in params:
            val=parval(parambase,p)
            if val in par2valcount[p]: par2valcount[p][val]+=1
            else: par2valcount[p][val]=1
    pardifflines=[]
    parsamelines=[]
    same=[]
    diff=[]
    for parname,valcount in par2valcount.items():
        if len(valcount)==1:
            same+=[parname]
            parsamelines+=['     SAME %s = %s [%i]'%(parname,list(valcount.keys())[0],list(valcount.values())[0])]
        else:
            diff+=[parname]
            vclines=[]
            for v,c in valcount.items():
                vclines+=['%s [%i]'%(str(v),c)]
            vclines.sort()
            pardifflines+=['     DIFF %s = %s'%(parname,' '.join(vclines))]
    if not ignore_same:
        for sameline in parsamelines:
            print(sameline)
    for diffline in pardifflines:
        print(diffline)
    return (same,diff)

def parvals(parambase_list,parname):
    vals=set()
    for pb in parambase_list:
        vals.add(pb.getParameter(parname))
    return vals

