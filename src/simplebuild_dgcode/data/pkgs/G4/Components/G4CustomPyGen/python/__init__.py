__all__=['GenBase']
import sys,os

from G4Interfaces.ToPy import Component as _Component
from . import _impl

#Particle indices (in the table in pymod__impl/mod.cc) and PDG codes, cached
#by particle name and by PDG code:
_particles_by_name = {}
_particles_by_pdg = {}

#Units, computed exactly as in CLHEP/Units/SystemOfUnits.h:
_meV = 1.e-3 * 1.e-6
_angstrom = 1.e-10 * 1000.

class G4PyGun:
    """Gun used to generate particles in G4CustomPyGen generators. Particles
    fired are collected and passed on to Geant4 after generate_event returns."""

    __slots__ = ( '_dir', '_empty_allowed', '_energy', '_idx', '_nfired', '_particles',
                  '_pdg', '_pos', '_time', '_weight' )

    def __init__( self ):
        self._particles = []
        self._nfired = 0
        self._empty_allowed = False
        self.set_type('neutron')
        self.set_energy(25*_meV)#fixme: set_wavelength_angstrom(1.8)
        self.set_position(0.0,0.0,0.0)
        self.set_direction(0.0,0.0,1.0)
        self._time = 0.0
        self._weight = 1.0

    def set_type( self, particle ):
        """Set the particle type (by name or PDG code)"""
        if isinstance( particle, str ):
            p = _particles_by_name.get( particle )
            if p is None:
                p = _impl.particle_by_name( particle )
                _particles_by_name[particle] = p
        else:
            pdg = int( particle )
            p = _particles_by_pdg.get( pdg )
            if p is None:
                p = ( _impl.particle_by_pdg( pdg ), pdg )
                _particles_by_pdg[pdg] = p
        self._idx, self._pdg = p

    def set_energy( self, e ):
        self._energy = float( e )

    def set_wavelength( self, wl ):
        if self._pdg != 2112:
            raise RuntimeError('G4CustomPyGen set_wavelength ERROR: set_wavelength method only supports neutrons so far.')
        #(negative values signal wavelengths to the C++ code):
        self._energy = -float( wl )

    def set_wavelength_angstrom( self, wl ):
        self.set_wavelength( wl * _angstrom )

    def set_position( self, x, y, z ):
        self._pos = ( float( x ), float( y ), float( z ) )

    def set_direction( self, dx, dy, dz ):
        self._dir = ( float( dx ), float( dy ), float( dz ) )

    def set_random_direction( self ):
        self._dir = _impl.rand_isotropic()

    def set_momentum( self, px, py, pz ):
        #NB: Only sets the direction (as always)
        self.set_direction( px, py, pz )

    def set_weight( self, w ):
        self._weight = float( w )

    def set_time( self, t ):
        self._time = float( t )

    def fire( self ):
        """Fire the gun (only needed when generating more than 1 particle per
        event or allow_empty_events() has been called)"""
        self._nfired += 1
        self._particles += ( self._idx, self._energy, *self._pos, *self._dir, self._time, self._weight )

    def allow_empty_events( self ):
        """Prevent the gun from firing automatically once when the generator
        did not fire it (needed if events can be empty)"""
        self._empty_allowed = True

    def _take_particles( self, aborted ):
        if not aborted:
            if not self._nfired and not self._empty_allowed:
                self.fire()
        elif self._nfired:
            raise RuntimeError('implementation error: Custom python generator aborted current event *after* it fired!')
        res = self._particles
        self._particles = []
        self._nfired = 0
        return res

class GenBase(_Component):
    """Base class for particle generators implemented in Python. Derived classes
    must implement generate_event(self,gun), and can optionally implement
    declare_parameters(self), init_generator(self,gun),
    validate_parameters(self) and unlimited(self)."""

    def __init__(self):
        super().__init__( _impl.create_handle() )
        object.__setattr__( self, '_g4gun', None )
        object.__setattr__( self, '_g4aborted', False )

        #Make sure we generate a proper name based on class name and file where it is implemented:
        self.setName(self.__construct_name())

        #if user implemented declare_parameters, call it:
        dp = getattr(self,'declare_parameters',None)
        if dp:
            dp()

        #Check if users added custom unlimited() on the python side:
        if type(self).unlimited is not _Component.unlimited:
            val = self.unlimited()
            if not isinstance(val,bool):
                raise ValueError('unlimited must return True or False (and the value must not change)')
            _impl.set_unlimited( self._g4handle, val )

        f = getattr(self,'generate_event',None)#required
        if not f:
            raise NotImplementedError("Must provide implementation of method generate_event(self,gun)")

        #Register callbacks, so the C++ side can call the methods at the appropriate time:
        validate = getattr(self,'validate_parameters',None)#optional
        initgen = getattr(self,'init_generator',None)#optional
        def cb_validate():
            return bool( validate() ) if validate else True
        def cb_initgen():
            self.__cache_parameters()
            gun = G4PyGun()
            object.__setattr__( self, '_g4gun', gun )
            if initgen:
                initgen( gun )
        def cb_genevt():
            gun = self._g4gun
            f( gun )
            return gun._take_particles( self._g4aborted )
        _impl.set_callbacks( self._g4handle, cb_validate, cb_initgen, cb_genevt )

    def __cache_parameters( self ):
        #Calls into the compiled code are relatively expensive, so once the
        #parameters are locked (which happens before the generator is
        #initialised, and after which they can not change), their values are
        #cached for fast access from generate_event (both as attributes and
        #via the getParameterXXX methods):
        if not self.isLocked():
            return
        cache = {}
        for t, names in ( ( 'dbl', self.getParameterListDouble() ), ( 'int', self.getParameterListInt() ),
                          ( 'flg', self.getParameterListBoolean() ), ( 'str', self.getParameterListString() ) ):
            for n in names:
                v = _Component.getParameter( self, n )
                cache[n] = ( t, v )
                if n not in self.__dict__ and not hasattr( type( self ), n ):
                    object.__setattr__( self, n, v )
        object.__setattr__( self, '_g4parcache', cache )

    def __cached( self, name, t ):
        c = self.__dict__.get( '_g4parcache' )
        if c is not None:
            e = c.get( name )
            if e is not None and e[0] == t:
                return e
        return None

    def getParameterDouble( self, name ):
        e = self.__cached( name, 'dbl' )
        return e[1] if e else _Component.getParameterDouble( self, name )

    def getParameterInt( self, name ):
        e = self.__cached( name, 'int' )
        return e[1] if e else _Component.getParameterInt( self, name )

    def getParameterBoolean( self, name ):
        e = self.__cached( name, 'flg' )
        return e[1] if e else _Component.getParameterBoolean( self, name )

    def getParameterString( self, name ):
        e = self.__cached( name, 'str' )
        return e[1] if e else _Component.getParameterString( self, name )

    def __construct_name(self):
        m = self.__module__
        if m == '__main__':
            m = os.path.basename(sys.argv[0])
        return os.path.join(m,self.__class__.__name__)

    def setName( self, name ):
        _impl.set_name( self._g4handle, name )

    def signalEndOfEvents( self, including_current_event ):
        """Signal that the generator has no more events (to be called from
        generate_event, with including_current_event=True if the current event
        can not be generated either)"""
        object.__setattr__( self, '_g4aborted', bool( including_current_event ) )
        _impl.signal_end_of_events( self._g4handle, bool( including_current_event ) )

    def addParameterDouble( self, name, value, lo = None, hi = None ):
        if lo is None and hi is None:
            _impl.add_parameter_double( self._g4handle, name, value )
        else:
            _impl.add_parameter_double_range( self._g4handle, name, value, lo, hi )

    def addParameterInt( self, name, value, lo = None, hi = None ):
        if lo is None and hi is None:
            _impl.add_parameter_int( self._g4handle, name, value )
        else:
            _impl.add_parameter_int_range( self._g4handle, name, value, lo, hi )

    def addParameterBoolean( self, name, value ):
        _impl.add_parameter_boolean( self._g4handle, name, bool( value ) )

    def addParameterString( self, name, value ):
        _impl.add_parameter_string( self._g4handle, name, value )

    def rand( self, a = None, b = None ):
        """Random number, flat in 0..1 (or a..b if given)"""
        if a is None and b is None:
            return _impl.rand( self._g4handle )
        return _impl.rand_range( self._g4handle, a, b )

    def randGauss( self, sigma = 1.0, mu = 0.0 ):
        return _impl.rand_gauss( self._g4handle, sigma, mu )

    def randExponential( self, lambda_ = 1.0 ):
        return _impl.rand_exponential( self._g4handle, lambda_ )

    def randPoisson( self, mu ):
        return _impl.rand_poisson( self._g4handle, mu )

    def randInt( self, a, b = None ):
        """Random integer in 0..a-1 (or a..b if b is given)"""
        if b is None:
            return _impl.rand_int( self._g4handle, a )
        return _impl.rand_int_range( self._g4handle, a, b )

    def randIsotropic( self ):
        """Random isotropic direction (as a tuple)"""
        return _impl.rand_isotropic()

    def create_hist_sampler(self,histstr):
        def badusage():
            print('ERROR: Bad create_hist_sampler arguments:')
            print('ERROR:    Please supply a single string of the form "FILENAME:HISTKEY:UNIT"')
            print('ERROR:    FILENAME should either be an absolute path to an .shist file,')
            print('ERROR:    or of the form "<pkgname>/<filename>.')
            print('ERROR:    HISTKEY should be the key of the chosen 1D histogram in the file')
            print('ERROR:    UNIT is optional and should be a number or a name from Units.units')
            raise ValueError('create_hist_sampler: bad arguments')

        parts=histstr.split(':')
        if len(parts) not in (2,3):
            badusage()
        filename=parts[0]
        if len(filename.split('/'))==2:
            import Core.FindData
            pkg,fn=filename.split('/',1)
            filename=Core.FindData(pkg,fn)
            if not os.path.exists(filename):
                raise ValueError('No data file named "%s" found in package "%s"'%(fn,pkg))
        else:
            if not filename or not os.path.exists(filename):
                raise ValueError('File not found: "%s"'%filename)
        if not filename.endswith('.shist'):
            raise ValueError('File not a SimpleHist collection (.shist): "%s"'%filename)
        histkey=parts[1]
        import SimpleHists
        hc=SimpleHists.HistCollection(filename)
        if not hc.hasKey(histkey):
            raise ValueError('Histogram with key "%s" not found in file "%s"'%(histkey,filename))
        h=hc.hist(histkey)
        unit = 1.0
        if len(parts)==3:
            try:
                unit = float(parts[2])
            except ValueError:
                unit = None
            if unit == None:
                import Units
                if hasattr(Units.units,parts[2]):
                    unit = getattr(Units.units,parts[2])
                    if not isinstance(unit,float):
                        unit = None
                if unit == None:
                    raise ValueError('Unknown unit: "%s"'%(parts[2]))
        assert isinstance(unit,float)
        import SimpleHistsUtils.Sampler
        s=SimpleHistsUtils.Sampler.Sampler(h,unit)
        def sampler():
            return s.sample(self.rand())
        return sampler
