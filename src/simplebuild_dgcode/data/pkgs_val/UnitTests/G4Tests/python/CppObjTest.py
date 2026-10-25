#Test module for G4Interfaces.ToPy.CppObject (see the testcppobjects script)
from G4Interfaces.ToPy import CppObject as _CppObject
from . import _CppObjTest as _m

def create_stepact():
    return _CppObject( _m.create_stepact() )

def create_evtact():
    return _CppObject( _m.create_evtact() )

def create_pregen():
    return _CppObject( _m.create_pregen() )

def create_postgen():
    return _CppObject( _m.create_postgen() )

def create_hook():
    return _CppObject( _m.create_hook() )

counts = _m.counts
