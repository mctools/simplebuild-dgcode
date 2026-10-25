"""Python side of framework components (geometries, particle generators and
step filters) provided by sbldpy modules (see the C++ headers
G4Interfaces/GeoConstructToPy.hh, G4Interfaces/ParticleGenToPy.hh and
G4Interfaces/StepFilterToPy.hh). The C++ objects are kept in a table in the
G4Interfaces library, and are referred to by integer handles, which are
wrapped in Component objects. Components provide the same Python interface as
the components provided by the older pybind11 based modules, and can be passed
to the G4Launcher in the same manner.

The Python module of a component typically just contains:

  from ._MyGeo import create_handle as _create_handle
  from G4Interfaces.ToPy import component_creator as _component_creator
  create = _component_creator( _create_handle )

"""

from . import _components as _c

__all__ = [ 'Component', 'CppObject', 'component_creator' ]

#FIXME: As a temporary migration measure, the Python interface of the Component
#class below (including parameters as attributes and swallowCmdLine) is to a
#large extent duplicated from the legacy pybind11 based code in
#Utils/python/ParametersBase.py (which adds those methods to the
#Utils._ParametersBase.ParametersBase class). It is duplicated rather than
#shared, so this module does not depend on any pybind11 based modules. When
#support for the pybind11 based components is removed, the legacy code in
#Utils/python/ParametersBase.py should be removed as well (and any fixes in
#the meantime should be applied to both places).

def _boolify_string( s ):
    assert s != '' and s is not None#this is for strings with content
    if s.lower() in ['y','yes','1','true','on']:
        return True
    if s.lower() in ['n','no','0','false','off']:
        return False
    print('Invalid value for boolean:',s)
    assert False
    return False

def _split_list( s ):
    return s.split('\n') if s else []

class Component:
    """Geometry, particle generator, or step filter provided by an sbldpy
    module. Parameters can be accessed as attributes."""

    def __init__( self, handle ):
        object.__setattr__( self, '_g4handle', handle )

    def getName( self ):
        return _c.get_name( self._g4handle )

    def dump( self, prefix = '' ):
        _c.dump( self._g4handle, prefix )

    def dumpNoLock( self, prefix = '' ):
        _c.dump_nolock( self._g4handle, prefix )

    def lock( self ):
        _c.lock( self._g4handle )

    def isLocked( self ):
        return _c.is_locked( self._g4handle )

    def setIgnoreRanges( self ):
        _c.set_ignore_ranges( self._g4handle )

    def noHardExitOnParameterFailure( self ):
        _c.no_hard_exit_on_parameter_failure( self._g4handle )

    def setParameterDouble( self, name, value ):
        _c.set_parameter_double( self._g4handle, name, value )

    def setParameterInt( self, name, value ):
        _c.set_parameter_int( self._g4handle, name, value )

    def setParameterBoolean( self, name, value ):
        _c.set_parameter_boolean( self._g4handle, name, bool( value ) )

    def setParameterString( self, name, value ):
        _c.set_parameter_string( self._g4handle, name, value )

    def getParameterDouble( self, name ):
        return _c.get_parameter_double( self._g4handle, name )

    def getParameterInt( self, name ):
        return _c.get_parameter_int( self._g4handle, name )

    def getParameterBoolean( self, name ):
        return _c.get_parameter_boolean( self._g4handle, name )

    def getParameterString( self, name ):
        return _c.get_parameter_string( self._g4handle, name )

    def hasParameterDouble( self, name ):
        return _c.has_parameter_double( self._g4handle, name )

    def hasParameterInt( self, name ):
        return _c.has_parameter_int( self._g4handle, name )

    def hasParameterBoolean( self, name ):
        return _c.has_parameter_boolean( self._g4handle, name )

    def hasParameterString( self, name ):
        return _c.has_parameter_string( self._g4handle, name )

    def getParameterListDouble( self ):
        return _split_list( _c.parameter_list_double( self._g4handle ) )

    def getParameterListInt( self ):
        return _split_list( _c.parameter_list_int( self._g4handle ) )

    def getParameterListBoolean( self ):
        return _split_list( _c.parameter_list_boolean( self._g4handle ) )

    def getParameterListString( self ):
        return _split_list( _c.parameter_list_string( self._g4handle ) )

    def getParameterList( self ):
        return sorted( self.getParameterListDouble() + self.getParameterListInt()
                       + self.getParameterListBoolean() + self.getParameterListString() )

    def getParameterTypes( self ):
        d = {}
        d.update( ( p, 'dbl' ) for p in self.getParameterListDouble() )
        d.update( ( p, 'int' ) for p in self.getParameterListInt() )
        d.update( ( p, 'flg' ) for p in self.getParameterListBoolean() )
        d.update( ( p, 'str' ) for p in self.getParameterListString() )
        return d

    def asDict( self ):
        d = {}
        for p in self.getParameterListDouble():
            d[p] = self.getParameterDouble( p )
        for p in self.getParameterListInt():
            d[p] = self.getParameterInt( p )
        for p in self.getParameterListBoolean():
            d[p] = self.getParameterBoolean( p )
        for p in self.getParameterListString():
            d[p] = self.getParameterString( p )
        return d

    def getParameter( self, name ):
        if self.hasParameterDouble( name ):
            return float( self.getParameterDouble( name ) )
        elif self.hasParameterInt( name ):
            return int( self.getParameterInt( name ) )
        elif self.hasParameterBoolean( name ):
            return bool( self.getParameterBoolean( name ) )
        elif self.hasParameterString( name ):
            return str( self.getParameterString( name ) )
        raise AttributeError( "%s has no '%s' attribute"%( self.__class__.__name__, name ) )

    def __getattr__( self, name ):
        if name.startswith( '__' ):
            raise AttributeError( name )
        return self.getParameter( name )

    def __setattr__( self, name, val ):
        if self.hasParameterDouble( name ):
            self.setParameterDouble( name, val )
        elif self.hasParameterInt( name ):
            self.setParameterInt( name, val )
        elif self.hasParameterBoolean( name ):
            self.setParameterBoolean( name, val )
        elif self.hasParameterString( name ):
            if isinstance( val, ( list, set, tuple ) ):
                val = ';'.join( val )
            self.setParameterString( name, val )
        else:
            #allow clients to add whatever attributes they want for other purposes:
            object.__setattr__( self, name, val )

    def __dir__( self ):
        return dir( self.__class__ ) + self.getParameterList()

    def tieParameters( self, name, other, other_name = None ):
        """Tie a parameter to a parameter on another component (or, if called
        with two parameter names, tie two parameters on this component)."""
        if other_name is None:
            _c.tie_parameters_local( self._g4handle, name, other )
        elif isinstance( other, Component ):
            _c.tie_parameters( self._g4handle, name, other._g4handle, other_name )
        else:
            self._legacy_object().tieParameters( name, other, other_name )

    def exposeParameter( self, other_name, other, new_name = '' ):
        """Expose a parameter from another component, possibly with a new name."""
        if isinstance( other, Component ):
            _c.expose_parameter( self._g4handle, other_name, other._g4handle, new_name )
        else:
            self._legacy_object().exposeParameter( other_name, other, new_name )

    def exposeParameters( self, other, prefix = '', tieOnClash = False ):
        """Expose all parameters from another component, possibly with a prefix
        on the new names."""
        if isinstance( other, Component ):
            _c.expose_parameters( self._g4handle, other._g4handle, prefix, bool( tieOnClash ) )
        else:
            self._legacy_object().exposeParameters( other, prefix, tieOnClash )

    def _legacy_object( self ):
        """Object referring to this component, of the classes in the legacy
        pybind11 based modules (only used when interacting with objects from
        those modules)."""
        from G4Interfaces._init import _legacy_component
        return _legacy_component( self._g4handle )

    def __repr__( self ):
        return '<%s %s (component handle %i)>'%( _c.kind( self._g4handle ), self.getName(), self._g4handle )

    def swallowCmdLine( self, args = None ):
        """Swallows command line arguments of the form parname=value"""
        #make sure that we actually modify the list instance in question:
        if args is None:
            import sys
            arglist = sys.argv
            iarg = 1
        else:
            arglist = args
            iarg = 0
        #special case for forcepars variable (also, do not consume it!)
        for a in arglist:
            if not a.startswith( 'forcepars=' ):
                continue
            a_split = a.split( '=' )
            if len( a_split ) == 2 and _boolify_string( a_split[1] ):
                self.setIgnoreRanges()
        while iarg < len( arglist ):
            a_split = arglist[iarg].split( '=', 1 )
            used = False
            if len( a_split ) == 2:
                name, val = a_split
                if name == 'forcepars':
                    iarg += 1
                    continue
                used = True
                if self.hasParameterDouble( name ):
                    self.setParameterDouble( name, float( val ) )
                elif self.hasParameterInt( name ):
                    self.setParameterInt( name, int( val ) )
                elif self.hasParameterBoolean( name ):
                    self.setParameterBoolean( name, _boolify_string( val ) )
                elif self.hasParameterString( name ):
                    self.setParameterString( name, val )
                else:
                    used = False
            if used:
                arglist.pop( iarg )
            else:
                iarg += 1

def _make_method( name, f ):
    def method( self, *args, **kwargs ):
        return f( self._g4handle, *args, **kwargs )
    method.__name__ = name
    method.__doc__ = f.__doc__
    return method

def component_creator( create_handle ):
    """Returns create() function for components provided by an sbldpy module
    (given the create_handle function of the module). Any functions named
    _method_<name> in the module (see G4Interfaces/ComponentToPy.hh) are added
    as methods <name> to the component objects, by creating them as instances
    of a subclass of Component."""
    import sys
    mod = sys.modules[ create_handle.__module__ ]
    methods = dict( ( k[len('_method_'):], _make_method( k[len('_method_'):], getattr( mod, k ) ) )
                    for k in dir( mod ) if k.startswith( '_method_' ) )
    cls = Component
    if methods:
        cls = type( 'Component', ( Component, ), methods )
        cls.__module__ = mod.__name__
        for k, m in methods.items():
            m.__qualname__ = 'Component.%s'%k
    def create():
        """Create new component"""
        return cls( create_handle() )
    return create

class CppObject:
    """C++ object (a Geant4 user action or a hook), created by an sbldpy module
    and referred to by a handle (see G4Interfaces/CppObjectHandles.hh). Such
    objects can be passed to for instance G4Launcher methods like
    setUserSteppingAction or addPostGenHook. Each object can only be used once."""

    __slots__ = ( '_g4objhandle', )

    def __init__( self, handle ):
        self._g4objhandle = handle

    def kind( self ):
        """Kind of object (e.g. "G4UserSteppingAction" or "PostGenCallBack")"""
        return _c.object_kind( self._g4objhandle )

    def isUsed( self ):
        """Whether the object was already used (it can only be used once)"""
        return _c.object_is_claimed( self._g4objhandle )

    def __repr__( self ):
        return '<C++ object %s%s>'%( self.kind(), ' (used)' if self.isUsed() else '' )
