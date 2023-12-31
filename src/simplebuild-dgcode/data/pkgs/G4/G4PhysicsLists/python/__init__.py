#autogenerated file
__doc__='python module for package G4PhysicsLists'
from . _init import *
custom_name_pattern = 'ESS_%s'

_custom = None
def _get_custom():
    global _custom
    if _custom!=None:
        return _custom
    _custom={}
    import pathlib
    import sysconfig
    import os
    pymoddir = pathlib.Path( os.environ['SBLD_INSTALL_PREFIX'] ) / 'python'
    assert pymoddir.is_dir()
    pyext_suffix = sysconfig.get_config_var('EXT_SUFFIX')
    for mod in pymoddir.glob('*/g4physlist_*%s'%pyext_suffix):
        bn = mod.name[0:-len(pyext_suffix)]
        packagename = mod.parent.name
        physlistname = custom_name_pattern%bn[len('g4physlist_'):]
        modulename = '%s.%s'%(packagename,bn)
        if physlistname in _custom:
            print("\n\nConfiguration ERROR: Multiple physics lists provided with the same name %s!!\n"%physlistname)
            print("                     They are located in the python modules:\n")
            print("                             %s"%modulename)
            print("                             %s\n"%_custom[physlistname][1])
            print("                     One of them needs to be renamed!!\n\n")
            import sys
            sys.exit(1)
        _custom[physlistname]=(packagename,modulename)
    return _custom

def getAllListNames(include_description = False, include_base = True, include_custom = True):
    base=[]
    custom=[]
    if include_base:
        base = getAllReferenceListNames()
        if include_description:
            base = [(b,'Geant4 reference list') for b in base]
    if include_custom:
        if include_description:
            custom = [(k,'List defined in package %s'%v[0]) for k,v in _get_custom().items()]
        else:
            custom = list(_get_custom().keys())
    return sorted(base)+sorted(custom)

def listIsCustom(plname):
    return plname in _get_custom()

def extractProvider(plname):
    _ = _get_custom().get(plname,None)
    if _ is None:
        raise RuntimeError('Unknown physics list: %s'%plname)
    pkg,modname = _
    import importlib
    return importlib.import_module(modname).create_provider(plname)

def printAvailableLists():
    print ("Available physics lists (append +TS or +OPTICAL to inject thermal scattering / optical processes):")
    for l,descr in getAllListNames(include_description=True):
        print ("   %s [%s]"%(l,descr))
