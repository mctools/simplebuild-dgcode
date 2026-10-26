"""Implementation of the GriffDataRead module, for reading .griff files. It
provides the same interface as the C++ classes in the GriffDataRead package
(see GriffDataRead/libinc/*.hh).

Implementation notes: All reading of files is done by a C++ GriffDataReader
(in the _impl module), which unpacks the data of the current event into a
buffer owned by the Python reader. The data is decoded here with the struct
module, only when needed (the layout of the data is defined by the writing
code in the G4DataCollect package, and by the reading code in the
GriffDataRead package, which must be kept consistent with the code here).
Steps are decoded per segment, when first requested. To avoid reference
cycles (which would require the garbage collector to free the objects of each
event), segments and steps refer to their track and segment via the reader and
indices, rather than directly. As in C++, objects from an event must not be
used after the reader moves to another event (their data is overwritten).
Shared data of the files (volume names, materials, particle definitions, ...)
is looked up in the C++ reader, and cached.
"""

import struct as _struct
import os as _os
import numpy as _np
from . import _impl
from math import copysign as _copysign
from Units import units as _u, constants as _c

__all__ = [ 'Element', 'FilterParams', 'GenParams', 'GeoParams', 'GriffDataReader', 'Isotope',
            'Material', 'Segment', 'Setup', 'Step', 'Track' ]

#Header at the start of the buffer, written by the _impl module (active flag,
#offset and size of the brief data, offset and size of the full data (-1 when
#not loaded), run number, event number, file index, event index in file, loop
#count, checksum, file format version, and a counter which changes whenever
#the reader moves to another event):
_s_bufhdr = _struct.Struct('<13q')
_FULLSIZE_OFFSET = 4 * 8
#Data layouts (all little endian):
_s_trkhdr = _struct.Struct('<QIII')#seed, metadata index, ntracks, mode
_s_trk = _struct.Struct('<iifIiII')#id, pdg, weight, creator process, parent id, nsegments, ndaughters
#Segment record (32 bytes) followed by the end time and energy (16 bytes),
#which are either stored separately (for the last segment on a track or when
#the next segment was filtered), or are the start values of the next segment:
_s_seg = _struct.Struct('<ddIiffdd')
#Step record (84 bytes: pre-step part of 68 bytes and other data of 16
#bytes) followed by the post-step part of 68 bytes, which is either stored
#separately (for the last step of a segment), or is the pre-step part of the
#next step:
_s_step = _struct.Struct('<5d6fI3fI5d6fI')
_s_u16 = _struct.Struct('<H')
_s_u32 = _struct.Struct('<I')
_SIZE_TRKHDR = 20
_SIZE_TRK = 28
_SIZE_SEG = 32
_SIZE_SEG_EXTRA = 16
_SIZE_STEPHDR = 8
_SIZE_STEP = 84
_trk_unpack = _s_trk.unpack_from
_seg_unpack = _s_seg.unpack_from
_step_unpack = _s_step.unpack_from
_u32_unpack = _s_u32.unpack_from
_daughter_structs = {}#cache of struct objects for daughter lists

#Sub-section IDs of the shared "database" data (see GriffFormat/Format.hh):
_DB_TOUCHABLES, _DB_VOLNAMES, _DB_MATERIALS, _DB_ELEMENTS, _DB_ISOTOPES = 100, 110, 120, 130, 140
_DB_MATNAMES, _DB_ELEMNAMES, _DB_ISONAMES, _DB_PROCNAMES = 150, 160, 170, 200
_DB_PDGCODES, _DB_PDGNAMES, _DB_PDGTYPES, _DB_PDGSUBTYPES = 300, 310, 320, 330
_DB_METADATA, _DB_METADATASTRINGS = 400, 410
_DB_STRINGS = ( _DB_VOLNAMES, _DB_MATNAMES, _DB_ELEMNAMES, _DB_ISONAMES, _DB_PROCNAMES,
                _DB_PDGNAMES, _DB_PDGTYPES, _DB_PDGSUBTYPES, _DB_METADATASTRINGS )

_modes = { 0 : 'FULL', 1 : 'REDUCED', 3 : 'MINIMAL' }
_stepstatus = ( 'WorldBoundary', 'GeomBoundary', 'AtRestDoItProc', 'AlongStepDoItProc',
                'PostStepDoItProc', 'UserDefinedLimit', 'ExclusivelyForcedProc', 'Undefined' )
_matstates = ( 'Undefined', 'Solid', 'Liquid', 'Gas' )

def _decode( b ):
    return b.decode( 'utf-8', errors = 'replace' )

class _Buf:
    """Sequential reading of binary data"""
    __slots__ = ( 'data', 'pos' )
    def __init__( self, data, pos = 0 ):
        self.data, self.pos = data, pos
    def unpack( self, s ):
        v = s.unpack_from( self.data, self.pos )
        self.pos += s.size
        return v
    def fmt( self, f ):
        return self.unpack( _struct.Struct( '<' + f ) )
    def string( self ):
        n, = self.unpack( _s_u16 )
        b = bytes( self.data[self.pos:self.pos+n] )
        self.pos += n
        return b

################################################################################
# Shared data (spread over the events of a file)                               #
################################################################################

class _Touchable:
    __slots__ = ( 'info', )
    def __init__( self, info ):
        self.info = info#(copy number, volume name, physical volume name, material) per depth

class _ParticleDefinition:
    #Fields in the order stored:
    _fields = ( 'mass', 'width', 'charge', 'lifeTime', 'nameIdx', 'typeIdx', 'subTypeIdx', 'pdgcode',
                'atomicNumber', 'atomicMass', 'magneticMoment', 'spin_halfs', 'stable', 'shortLived' )
    __slots__ = ( 'atomicMass', 'atomicNumber', 'charge', 'lifeTime', 'magneticMoment', 'mass', 'nameIdx',
                  'pdgcode', 'shortLived', 'spin_halfs', 'stable', 'subTypeIdx', 'typeIdx', 'width' )
    def __init__( self, vals ):
        for k, v in zip( self._fields, vals ):
            setattr( self, k, v )

class Isotope:
    """Isotope in a material (see GriffDataRead/Isotope.hh)"""
    def __init__( self, dr, info ):
        self._dr = dr
        self._nameIdx, self._Z, self._N, self._A, self._m = info
    def getName( self ):
        return self._dr._strings[_DB_ISONAMES][self._nameIdx]
    def Z( self ):
        return self._Z
    def N( self ):
        return self._N
    def A( self ):
        return self._A
    def m( self ):
        return self._m
    def transient_id( self ):
        return id( self )
    def dump( self ):
        print( '[isotope: name=%s Z=%i N=%i A=%fg/mole m=%i]'%( self.getName(), self._Z, self._N,
                                                                self._A/( _u.g/_u.mole ), self._m ), end = '' )

class Element:
    """Element in a material (see GriffDataRead/Element.hh)"""
    def __init__( self, dr, info, isos ):
        self._dr = dr
        self._nameIdx, self._symbolIdx, self._Z, self._N, self._A, self._natural = info
        self._isos = _pairs( isos )#(relative abundance, index)
    def getName( self ):
        return self._dr._strings[_DB_ELEMNAMES][self._nameIdx]
    def getSymbol( self ):
        return self._dr._strings[_DB_ELEMNAMES][self._symbolIdx]
    def Z( self ):
        return self._Z
    def N( self ):
        return self._N
    def A( self ):
        return self._A
    def naturalAbundances( self ):
        return self._natural
    def numberIsotopes( self ):
        return len( self._isos )
    def isotopeRelativeAbundance( self, i ):
        return self._isos[i][0]
    def getIsotope( self, i ):
        return self._dr._db[_DB_ISOTOPES][self._isos[i][1]]
    @property
    def isotopes( self ):
        return [ self.getIsotope( i ) for i in range( len( self._isos ) ) ]
    def transient_id( self ):
        return id( self )
    def dump( self ):
        s = '[element: name=%s symbol=%s Z=%f N=%f A=%fg/mole naturalabundance=%s'%(
            self.getName(), self.getSymbol(), self._Z, self._N, self._A/( _u.g/_u.mole ),
            'yes' if self._natural else 'no' )
        for i in range( len( self._isos ) ):
            iso = self.getIsotope( i )
            s += ' iso%i[%f%%]=%s(%i,%i)'%( i, 100.0*self._isos[i][0], iso.getName(), iso.Z(), iso.N() )
        print( s + ']', end = '' )

class Material:
    """Material of a volume (see GriffDataRead/Material.hh)"""
    def __init__( self, dr, info, elems ):
        self._dr = dr
        ( self._nameIdx, self._density, self._temperature, self._pressure, self._radlen,
          self._nuclintlen, self._imean, self._state ) = info
        self._elems = _pairs( elems )#(fraction, index)
    def getName( self ):
        return self._dr._strings[_DB_MATNAMES][self._nameIdx]
    def density( self ):
        return self._density
    def temperature( self ):
        return self._temperature
    def pressure( self ):
        return self._pressure
    def radiationLength( self ):
        return self._radlen
    def nuclearInteractionLength( self ):
        return self._nuclintlen
    def hasMeanExitationEnergy( self ):
        return self._imean != -1
    def meanExitationEnergy( self ):
        return self._imean
    def isSolid( self ):
        return self._state == 1
    def isLiquid( self ):
        return self._state == 2
    def isGas( self ):
        return self._state == 3
    def isUndefinedState( self ):
        return self._state == 0
    def stateStr( self ):
        return _matstates[self._state]
    def numberElements( self ):
        return len( self._elems )
    def elementFraction( self, i ):
        return self._elems[i][0]
    def getElement( self, i ):
        return self._dr._db[_DB_ELEMENTS][self._elems[i][1]]
    @property
    def elements( self ):
        return [ self.getElement( i ) for i in range( len( self._elems ) ) ]
    def transient_id( self ):
        return id( self )
    def dump( self ):
        s = ( '[material: name=%s density=%fg/cm3 temp=%fK pressure=%fbar radlen=%fmm nuclintlen=%fmm'
              ' Imean=%feV state=%s' )%( self.getName(), self._density/( _u.g/_u.cm3 ), self._temperature/_u.kelvin,
                                         self._pressure/_u.bar, self._radlen/_u.mm, self._nuclintlen/_u.mm,
                                         ( self._imean/_u.eV if self.hasMeanExitationEnergy() else -1 ),
                                         self.stateStr() )
        for i in range( len( self._elems ) ):
            s += ' elem%i[%f%%]=%s'%( i, 100.0*self._elems[i][0], self.getElement( i ).getName() )
        print( s + ']', end = '' )

def _pairs( a ):
    #(float, index) pairs from flat numpy array:
    return [ ( float( a[i] ), int( a[i+1] ) ) for i in range( 0, len( a ), 2 ) ]

class _Lookup( dict ):
    """Cache of shared data, filled on demand"""
    __slots__ = ( '_fct', )
    def __init__( self, fct ):
        self._fct = fct
    def __missing__( self, key ):
        v = self[key] = self._fct( key )
        return v

################################################################################
# Setup                                                                        #
################################################################################

class _Params:
    """Named parameters of a geometry, generator or filter used in the
    simulation (read-only). Parameters can be accessed as attributes."""

    _kindname = ''

    def __init__( self, data, name ):
        object.__setattr__( self, '_name', name )
        pars = {}
        buf = _Buf( data )
        npars, = buf.unpack( _s_u32 )
        for _ in range( npars ):
            pname = _decode( buf.string() )
            t, = buf.fmt( 'B' )
            if t == 0:
                pars[pname] = ( 'dbl', buf.fmt( 'd' )[0] )
            elif t == 1:
                pars[pname] = ( 'int', buf.fmt( 'i' )[0] )
            elif t == 2:
                pars[pname] = ( 'flg', bool( buf.fmt( 'B' )[0] ) )
            elif t == 3:
                pars[pname] = ( 'str', _decode( buf.string() ) )
            else:
                raise RuntimeError( 'Invalid parameter data in Griff file' )
        object.__setattr__( self, '_pars', pars )#(insertion order is kept for dumps)

    def __setattr__( self, name, val ):
        raise AttributeError( 'Parameters from Griff files can not be modified' )

    def getName( self ):
        return self._name

    def _get( self, name, t ):
        e = self._pars.get( name )
        if e is None or e[0] != t:
            raise RuntimeError( 'Parameters ERROR: No %s parameter named %s'%( t, name ) )
        return e[1]
    def getParameterDouble( self, name ):
        return self._get( name, 'dbl' )
    def getParameterInt( self, name ):
        return self._get( name, 'int' )
    def getParameterBoolean( self, name ):
        return self._get( name, 'flg' )
    def getParameterString( self, name ):
        return self._get( name, 'str' )

    def _has( self, name, t ):
        e = self._pars.get( name )
        return e is not None and e[0] == t
    def hasParameterDouble( self, name ):
        return self._has( name, 'dbl' )
    def hasParameterInt( self, name ):
        return self._has( name, 'int' )
    def hasParameterBoolean( self, name ):
        return self._has( name, 'flg' )
    def hasParameterString( self, name ):
        return self._has( name, 'str' )

    def _list( self, t ):
        return sorted( k for k, e in self._pars.items() if e[0] == t )
    def getParameterListDouble( self ):
        return self._list( 'dbl' )
    def getParameterListInt( self ):
        return self._list( 'int' )
    def getParameterListBoolean( self ):
        return self._list( 'flg' )
    def getParameterListString( self ):
        return self._list( 'str' )
    def getParameterList( self ):
        return sorted( self._pars )
    def getParameterTypes( self ):
        return dict( ( k, e[0] ) for k, e in self._pars.items() )
    def asDict( self ):
        d = {}
        for t in ( 'dbl', 'int', 'flg', 'str' ):
            for k in self._list( t ):
                d[k] = self._pars[k][1]
        return d
    def getParameter( self, name ):
        e = self._pars.get( name )
        if e is None:
            raise AttributeError( "%s has no '%s' attribute"%( self.__class__.__name__, name ) )
        return e[1]
    def isLocked( self ):
        return True
    def __getattr__( self, name ):
        if name.startswith( '_' ):
            raise AttributeError( name )
        return self.getParameter( name )
    def __dir__( self ):
        return dir( self.__class__ ) + sorted( self._pars )

    def dumpNoLock( self, prefix = '' ):
        for k, ( t, v ) in self._pars.items():
            if t == 'dbl':
                print( '%s[dbl] %s = %.14g'%( prefix, k, v ) )
            elif t == 'int':
                print( '%s[int] %s = %i'%( prefix, k, v ) )
            elif t == 'flg':
                print( '%s[flg] %s = %s'%( prefix, k, 'yes' if v else 'no' ) )
            else:
                print( '%s[str] %s = "%s"'%( prefix, k, v ) )

    def dump( self, prefix = '' ):
        print( '%s%s[%s]:'%( prefix, self._kindname, self._name ) )
        self.dumpNoLock( prefix + '  ' )

class GeoParams( _Params ):
    _kindname = 'GeoConstructor'

class GenParams( _Params ):
    _kindname = 'ParticleGenerator'

class FilterParams( _Params ):
    _kindname = 'StepFilter'

class Setup:
    """Setup used for the simulation (see GriffDataRead/Setup.hh)"""

    def __init__( self, alldata ):
        self._all = alldata#dict str->bytes (values can be binary data)
        self._cache = {}

    def _binary( self, key ):
        return self._all.get( '`' + key )

    #Reference counting is not needed in Python (kept for compatibility):
    def ref( self ):
        pass

    def unref( self ):
        pass

    def refCount( self ):
        return 1

    def _allData( self ):
        return self._all

    def metaData( self ):
        """Standard metadata (dict)"""
        return dict( ( k, _decode( v ) ) for k, v in self._all.items() if k[0] not in '^`' )

    def userData( self ):
        """Custom metadata (dict)"""
        return dict( ( k[1:], _decode( v ) ) for k, v in self._all.items() if k[0] == '^' )

    def _params( self, cls, what ):
        if what not in self._cache:
            data = self._binary( what + 'Serialised' )
            name = self._all.get( what + 'Name' )
            self._cache[what] = None if ( data is None or name is None ) else cls( data, _decode( name ) )
        return self._cache[what]

    def geo( self ):
        """Geometry parameters"""
        return self._params( GeoParams, 'geo' )

    def gen( self ):
        """Particle generator parameters"""
        return self._params( GenParams, 'gen' )

    def hasFilter( self ):
        return self._binary( 'filterSerialised' ) is not None

    def getFilter( self ):
        """Step filter parameters"""
        return self._params( FilterParams, 'filter' )

    def hasKillFilter( self ):
        return self._binary( 'killFilterSerialised' ) is not None

    def killFilter( self ):
        """Kill filter parameters"""
        return self._params( FilterParams, 'killFilter' )

    def cmds( self ):
        """Geant4 commands (list)"""
        data = self._binary( 'g4Cmds' )
        if data is None:
            return []
        buf = _Buf( data )
        n, = buf.unpack( _s_u16 )
        return [ _decode( buf.string() ) for _ in range( n ) ]

    def dump( self, prefix = '' ):
        print( '%s===========================  GRIFF SETUP  ==========================='%prefix )
        for title, d in ( ( 'UserData', self.userData() ), ( 'MetaData', self.metaData() ) ):
            print( '%s  %s:'%( prefix, title ) )
            if not d:
                print( '%s    <none>'%prefix )
            for k in sorted( d ):
                print( '%s    "%s" : "%s"'%( prefix, k, d[k] ) )
        print( '%s  Geant4 Commands:'%prefix )
        cmds = self.cmds()
        if not cmds:
            print( '%s    <none>'%prefix )
        for c in cmds:
            print( '%s    "%s"'%( prefix, c ) )
        p = prefix + '  '
        self.geo().dump( p )
        self.gen().dump( p )
        if self.hasFilter():
            self.getFilter().dump( p )
        if self.hasKillFilter():
            print( '%sKillFilter:'%p )
            self.killFilter().dump( p + '  ' )
        print( '%s====================================================================='%prefix )

################################################################################
# Event data                                                                   #
################################################################################

class Step:
    """Step on a segment (see GriffDataRead/Step.hh)"""

    __slots__ = ( '_dr', '_i', '_is', '_it', '_v' )

    def __init__( self, dr, it, iseg, v, i ):
        self._dr, self._it, self._is, self._v, self._i = dr, it, iseg, v, i

    def getTrack( self ):
        return self._dr._tracks[self._it]
    def getSegment( self ):
        return self._dr._tracks[self._it]._sg()[self._is]
    def iStep( self ):
        return self._i
    def getNextStep( self ):
        s = self.getSegment()._steps()
        return s[self._i+1] if self._i + 1 < len( s ) else None
    def getPreviousStep( self ):
        return self.getSegment()._steps()[self._i-1] if self._i else None
    def eDep( self ):
        return self._v[12]
    def eDepNonIonising( self ):
        return self._v[13]
    def stepLength( self ):
        return self._v[14]
    def stepStatus( self ):
        return self._v[15]
    def stepStatusStr( self ):
        return _stepstatus[self._v[15]]
    def preGlobalX( self ):
        return self._v[0]
    def preGlobalY( self ):
        return self._v[1]
    def preGlobalZ( self ):
        return self._v[2]
    def preGlobal( self ):
        return self._v[0:3]
    def preTime( self ):
        return self._v[3]
    def preEKin( self ):
        return abs( self._v[4] )
    def preAtVolEdge( self ):
        #NB: The sign of the stored energy indicates if the point is at an edge:
        return _copysign( 1.0, self._v[4] ) < 0
    def preLocalX( self ):
        return self._v[5]
    def preLocalY( self ):
        return self._v[6]
    def preLocalZ( self ):
        return self._v[7]
    def preLocal( self ):
        return self._v[5:8]
    def preMomentumX( self ):
        return self._v[8]
    def preMomentumY( self ):
        return self._v[9]
    def preMomentumZ( self ):
        return self._v[10]
    def preMomentum( self ):
        return self._v[8:11]
    def preProcessDefinedStep( self ):
        return self._dr._strings[_DB_PROCNAMES][self._v[11]]
    def postGlobalX( self ):
        return self._v[16]
    def postGlobalY( self ):
        return self._v[17]
    def postGlobalZ( self ):
        return self._v[18]
    def postGlobal( self ):
        return self._v[16:19]
    def postTime( self ):
        return self._v[19]
    def postEKin( self ):
        return abs( self._v[20] )
    def postAtVolEdge( self ):
        return _copysign( 1.0, self._v[20] ) < 0
    def postLocalX( self ):
        return self._v[21]
    def postLocalY( self ):
        return self._v[22]
    def postLocalZ( self ):
        return self._v[23]
    def postLocal( self ):
        return self._v[21:24]
    def postMomentumX( self ):
        return self._v[24]
    def postMomentumY( self ):
        return self._v[25]
    def postMomentumZ( self ):
        return self._v[26]
    def postMomentum( self ):
        return self._v[24:27]
    def postProcessDefinedStep( self ):
        return self._dr._strings[_DB_PROCNAMES][self._v[27]]

    def dump( self ):
        v, ue, ut, ul = self._v, _u.keV, _u.ms, _u.mm
        print( ( '[step(units:ms,mm,keV): edep=%f edepnonion=%f stepLength=%f stepStatus=%s t0=%f t1=%f'
                 ' ekin0=%f ekin1=%f x0=%f y0=%f z0=%f x1=%f y1=%f z1=%f lx0=%f ly0=%f lz0=%f lx1=%f ly1=%f lz1=%f'
                 ' edge0=%i edge1=%i px0=%f py0=%f pz0=%f px1=%f py1=%f pz1=%f proc0=%s proc1=%s]' )%(
                     v[12]/ue, v[13]/ue, v[14]/ul, self.stepStatusStr(), v[3]/ut, v[19]/ut,
                     abs( v[4] )/ue, abs( v[20] )/ue, v[0]/ul, v[1]/ul, v[2]/ul, v[16]/ul, v[17]/ul, v[18]/ul,
                     v[5]/ul, v[6]/ul, v[7]/ul, v[21]/ul, v[22]/ul, v[23]/ul,
                     self.preAtVolEdge(), self.postAtVolEdge(),
                     v[8]/ue, v[9]/ue, v[10]/ue, v[24]/ue, v[25]/ue, v[26]/ue,
                     self.preProcessDefinedStep(), self.postProcessDefinedStep() ), end = '' )

class Segment:
    """Segment of a track inside a given volume (see GriffDataRead/Segment.hh)"""

    __slots__ = ( '_dr', '_i', '_it', '_stepcache', '_v' )

    def __init__( self, dr, it, v, i ):
        self._dr, self._it, self._v, self._i, self._stepcache = dr, it, v, i, None

    def getTrack( self ):
        return self._dr._tracks[self._it]
    def iSegment( self ):
        return self._i
    def getNextSegment( self ):
        s = self.getTrack()._sg()
        return s[self._i+1] if self._i + 1 < len( s ) else None
    def getPreviousSegment( self ):
        return self.getTrack()._sg()[self._i-1] if self._i else None
    def nextWasFiltered( self ):
        return bool( self._v[2] & 0x20000000 )
    def startTime( self ):
        return self._v[0]
    def endTime( self ):
        return self._v[6]
    def startEKin( self ):
        return self._v[1]
    def endEKin( self ):
        return self._v[7]
    def eDep( self ):
        return self._v[4]
    def eDepNonIonising( self ):
        return self._v[5]
    def startAtVolumeBoundary( self ):
        return bool( self._v[2] & 0x40000000 )
    def endAtVolumeBoundary( self ):
        return bool( self._v[2] & 0x80000000 )

    def _touchable( self ):
        try:
            return self._dr._touchables[self._v[2] & 0x1FFFFFFF]
        except IndexError:
            return self._dr._loadTouchables( self._v[2] & 0x1FFFFFFF )
    def volumeDepthStored( self ):
        return len( self._touchable().info )
    def volumeName( self, idepth = 0 ):
        i = self._touchable().info[idepth][1]
        try:
            return self._dr._volnames[i]
        except KeyError:
            return self._dr._strings[_DB_VOLNAMES][i]
    def physicalVolumeName( self, idepth = 0 ):
        return self._dr._strings[_DB_VOLNAMES][self._touchable().info[idepth][2]]
    def volumeCopyNumber( self, idepth = 0 ):
        return self._touchable().info[idepth][0]
    def material( self, idepth = 0 ):
        return self._dr._db[_DB_MATERIALS][self._touchable().info[idepth][3]]
    def isInWorldVolume( self ):
        return self.volumeDepthStored() == 1
    def inSameVolume( self, other ):
        return ( self._v[2] & 0x1FFFFFFF ) == ( other._v[2] & 0x1FFFFFFF )

    def _steps( self ):
        s = self._stepcache
        if s is None:
            s = self._stepcache = self._dr._loadSteps( self )
        return s
    def nStepsOriginal( self ):
        raw = self._v[3]
        if raw < 0:
            return -raw
        return _u32_unpack( self._dr._fulldata(), raw )[0]
    def nStepsStored( self ):
        return len( self._steps() )
    def hasStepInfo( self ):
        return self._v[3] >= 0
    def getStep( self, i ):
        return self._steps()[i]
    def stepBegin( self ):
        return self._steps()[0]
    def firstStep( self ):
        return self._steps()[0]
    def lastStep( self ):
        return self._steps()[-1]
    @property
    def steps( self ):
        s = self._stepcache
        if s is None:
            s = self._stepcache = self._dr._loadSteps( self )
        return iter( s )
    def segmentLength( self ):
        return sum( s._v[14] for s in self._steps() )

    def dump( self ):
        ue, ut = _u.keV, _u.ms
        v = self._v
        s = ( '[segment(units:ms,keV): t0=%f t1=%f ekin0=%f ekin1=%f edep=%f edepnonion=%f edge0=%i edge1=%i' )%(
            v[0]/ut, v[6]/ut, v[1]/ue, v[7]/ue, v[4]/ue, v[5]/ue, self.startAtVolumeBoundary(),
            self.endAtVolumeBoundary() )
        for i in range( self.volumeDepthStored() ):
            s += ' v%i=%s#%s#%s#%i'%( i, self.volumeName( i ), self.physicalVolumeName( i ),
                                      self.material( i ).getName(), self.volumeCopyNumber( i ) )
        print( s + ']', end = '' )

class Track:
    """Track of a particle (see GriffDataRead/Track.hh)"""

    __slots__ = ( '_daughters', '_dr', '_it', '_segs', '_v' )

    def __init__( self, dr, it, v, daughters ):
        self._dr, self._it, self._v, self._daughters, self._segs = dr, it, v, daughters, None

    def _sg( self ):
        #Segments are decoded when first needed:
        s = self._segs
        if s is None:
            s = self._segs = self._dr._loadSegments( self )
        return s

    def _firstSegValues( self ):
        s = self._segs
        if s is not None:
            return s[0]._v
        return self._dr._firstSegValues( self )

    def getDataReader( self ):
        return self._dr
    def trackID( self ):
        return self._v[0]
    def parentID( self ):
        return self._v[4]
    def getParent( self ):
        return self._dr.getTrackByID( self._v[4] ) if self._v[4] else None
    def nDaughters( self ):
        return len( self._daughters )
    def getDaughterID( self, i ):
        return self._daughters[i]
    def getDaughter( self, i ):
        return self._dr.getTrackByID( self._daughters[i] )
    @property
    def daughters( self ):
        return ( self._dr.getTrackByID( d ) for d in self._daughters )
    def nSegments( self ):
        return self._v[5]
    def getSegment( self, i ):
        return self._sg()[i]
    @property
    def segments( self ):
        s = self._segs
        if s is None:
            s = self._segs = self._dr._loadSegments( self )
        return iter( s )
    def segmentBegin( self ):
        return self._sg()[0]
    def firstSegment( self ):
        return self._sg()[0]
    def lastSegment( self ):
        return self._sg()[-1]
    def firstStep( self ):
        return self._sg()[0].firstStep()
    def lastStep( self ):
        return self._sg()[-1].lastStep()
    def weight( self ):
        return self._v[2]
    def isPrimary( self ):
        return self._v[4] == 0
    def isSecondary( self ):
        return self._v[4] != 0
    def startTime( self ):
        return self._firstSegValues()[0]
    def startEKin( self ):
        return self._firstSegValues()[1]
    def creatorProcess( self ):
        return self._dr._strings[_DB_PROCNAMES][self._v[3]]
    def pdgCode( self ):
        return self._v[1]
    def _pdef( self ):
        return self._dr._pdefs[self._v[1]]
    def pdgName( self ):
        return self._dr._strings[_DB_PDGNAMES][self._pdef().nameIdx]
    def pdgType( self ):
        return self._dr._strings[_DB_PDGTYPES][self._pdef().typeIdx]
    def pdgSubType( self ):
        return self._dr._strings[_DB_PDGSUBTYPES][self._pdef().subTypeIdx]
    def mass( self ):
        return self._pdef().mass
    def width( self ):
        return self._pdef().width
    def charge( self ):
        return self._pdef().charge
    def lifeTime( self ):
        return self._pdef().lifeTime
    def atomicNumber( self ):
        return self._pdef().atomicNumber
    def atomicMass( self ):
        return self._pdef().atomicMass
    def magneticMoment( self ):
        return self._pdef().magneticMoment
    def spin( self ):
        return self._pdef().spin_halfs * 0.5
    def stable( self ):
        return bool( self._pdef().stable )
    def shortLived( self ):
        return bool( self._pdef().shortLived )

    def dump( self, dumpPDGInfo = False ):
        ue, ut = _u.keV, _u.ms
        s = '[track(units:ms,keV): id=%i parent=%i'%( self._v[0], self._v[4] )
        for i, d in enumerate( self._daughters ):
            s += ' daug%i=%i'%( i, d )
        s += ' nsegm=%i w=%f primary=%i secondary=%i t0[ms]=%f ekin0[keV]=%f pdg=%i proc=%s'%(
            self._v[5], self._v[2], self.isPrimary(), self.isSecondary(), self.startTime()/ut,
            self.startEKin()/ue, self._v[1], self.creatorProcess() )
        if dumpPDGInfo:
            nuclearMagneton = ( _c.h_Planck/( 4*_c.pi ) )/( _c.proton_mass_c2/_c.c_squared )
            s += ( ' name=%s type=%s subtype=%s mass=%f width=%f charge=%f lifeTime=%f atomicNumber=%i'
                   ' atomicMass=%i magneticMoment[muN]=%f spin=%f stable=%i shortLived=%i' )%(
                       self.pdgName(), self.pdgType(), self.pdgSubType(), self.mass()/ue, self.width()/ue,
                       self.charge(), self.lifeTime()/ut, self.atomicNumber(), self.atomicMass(),
                       self.magneticMoment()/nuclearMagneton, self.spin(), self.stable(), self.shortLived() )
        print( s, end = '' )

    def dump_full( self ):
        self.dump( True )

################################################################################
# Reading of files                                                             #
################################################################################

_openMsg = [ True ]
_impl.set_open_msg( False )#(the message is printed here instead)

class GriffDataReader:
    """Reader of .griff files (see GriffDataRead/GriffDataReader.hh). The input
    file name can contain wildcards, or a list of files can be given. Use
    nloops > 1 to loop through the files several times (0 means forever). The
    data of each event is unpacked into a buffer of buffer_mb megabytes (an
    error is raised for events which do not fit), which also determines the
    size of the buffers used by the iterators in the GriffAnaUtils module."""

    @staticmethod
    def setOpenMsg( b ):
        """Enable/disable the message printed when opening a file"""
        _openMsg[0] = bool( b )

    @staticmethod
    def openMsg():
        return _openMsg[0]

    def __init__( self, inputFiles, nloops = 1, buffer_mb = 20 ):
        import glob
        patterns = [ inputFiles ] if isinstance( inputFiles, ( str, _os.PathLike ) ) else list( inputFiles )
        files = []
        for p in patterns:
            p = str( p )
            files += sorted( glob.glob( p ) ) if '*' in p else [ p ]
        if not files:
            raise RuntimeError( 'GriffDataReader::ERROR No files found based on specified input file patterns!'
                                if patterns else 'GriffDataReader::ERROR No input files specified!' )
        for f in files:
            if not _os.path.exists( f ):
                raise RuntimeError( 'GriffDataReader::ERROR Input file does not exist: %s'%f )
            if '\n' in f:
                raise RuntimeError( 'GriffDataReader::ERROR Unsupported file name: %r'%f )
        nloops = int( nloops )
        if nloops < 0:
            raise ValueError( 'nloops must not be negative' )
        self._files = files
        self._buffer_mb = float( buffer_mb )
        self._buf = _np.empty( int( self._buffer_mb * 1024 * 1024 ), dtype = _np.uint8 )
        self._mv = memoryview( self._buf )
        self._allowSetupChange = False
        self._cachedMetaData = None#(fileIdx, metadata index, metadata map) for consistency checks
        self._lastSetup = None#(fileIdx, metadata index, Setup object) for setup() and setupChanged()
        self._dbFileIdx = None
        self._openedFile = None
        self._clearEvent()
        self._r = _impl.Reader( '\n'.join( files ), nloops, self._buf )
        self._rh = self._r.handle()#handle of the C++ reader (for the GriffAnaUtils module)
        self._newEvent( True )

    ######################
    # Shared data        #
    ######################

    def _clearDB( self ):
        r = self._r
        self._strings = dict( ( k, _Lookup( lambda i, k = k : r.db_string( k, i ) ) ) for k in _DB_STRINGS
                              if k != _DB_METADATASTRINGS )
        self._touchables = []
        self._volnames = {}#plain dict for fast access to cached volume names
        self._strings[_DB_VOLNAMES] = _Lookup( lambda i : self._volnames.setdefault( i, r.db_string( _DB_VOLNAMES, i ) ) )
        self._db = { _DB_MATERIALS : _Lookup( lambda i : Material( self, r.material( i ), r.material_elements( i ) ) ),
                     _DB_ELEMENTS : _Lookup( lambda i : Element( self, r.element( i ), r.element_isotopes( i ) ) ),
                     _DB_ISOTOPES : _Lookup( lambda i : Isotope( self, r.isotope( i ) ) ) }
        self._pdefs = _Lookup( lambda pdg : _ParticleDefinition( r.particle_definition( pdg ) ) )

    def _loadTouchables( self, i ):
        #Load all touchables available, in one go:
        t = self._touchables
        vals = self._r.touchables( len( t ) ).tolist()
        pos, n = 0, len( vals )
        while pos < n:
            depth = vals[pos]
            t.append( _Touchable( [ tuple( vals[k:k+4] ) for k in range( pos + 1, pos + 1 + 4 * depth, 4 ) ] ) )
            pos += 1 + 4 * depth
        return t[i]

    def _metaDataMap( self, mdidx ):
        data = self._r.metadata( mdidx ).tobytes()
        res, pos, n = {}, 0, len( data )
        while pos < n:
            kv = []
            for _ in range( 2 ):
                k, = _u32_unpack( data, pos )
                kv.append( data[pos+4:pos+4+k] )
                pos += 4 + k
            res[_decode( kv[0] )] = kv[1]
        return res

    ######################
    # Navigation         #
    ######################

    def _clearEvent( self ):
        self._tracks = None
        self._brief = None
        self._full = None
        self._hdr = None

    def _newEvent( self, ok ):
        #Called after any navigation (the C++ reader has updated the buffer):
        self._clearEvent()
        h = self._h = _s_bufhdr.unpack_from( self._buf, 0 )
        if h[0]:
            fileidx = h[7]
            if fileidx != self._dbFileIdx:
                self._dbFileIdx = fileidx
                self._clearDB()
                fn = self._files[fileidx]
                if fn != self._openedFile:
                    self._openedFile = fn
                    if self.openMsg():
                        print( 'GriffDataReader opened file %s'%_os.path.basename( fn ) )
            if not self._allowSetupChange:
                self._checkSetupConsistency()
        return ok

    def _checkSetupConsistency( self ):
        mdidx = self._metaDataIdx()
        c = self._cachedMetaData
        fileidx = self._h[7]
        if c is None:
            self._cachedMetaData = ( fileidx, mdidx, self._metaDataMap( mdidx ) )
            return
        if fileidx != c[0]:
            ok = c[2] == self._metaDataMap( mdidx )
            if ok:
                self._cachedMetaData = ( fileidx, mdidx, c[2] )
        else:
            ok = mdidx == c[1]
        if not ok:
            raise RuntimeError( 'GriffDataReader ERROR: Two events in the input file(s) were simulated using'
                                ' different setups. If you actually wish to analyse events from multiple setups,'
                                ' then please call allowSetupChange() on the reader before looping over events.' )

    def eventActive( self ):
        return self._h[0] != 0

    def goToFirstEvent( self ):
        """Go to the first event of the first file with events"""
        return self._newEvent( self._r.go_to_first_event() )

    def goToNextFile( self ):
        """Go to the first event in the next file"""
        return self._newEvent( self._r.go_to_next_file() )

    def goToNextEvent( self ):
        """Go to the next event (in the next file if needed)"""
        return self._newEvent( self._r.go_to_next_event() )

    def skipEvents( self, n ):
        return self._newEvent( self._r.skip_events( n ) )

    def loopEvents( self ):
        """For looping through all events: while dr.loopEvents(): ... (reset
        afterwards with goToFirstEvent())"""
        return self._newEvent( self._r.loop_events() )

    def loopCount( self ):
        return self._h[9]

    def seekEventByIndexInCurrentFile( self, idx ):
        return self._newEvent( self._r.seek_event( idx ) )

    def eventIndexInCurrentFile( self ):
        return self._h[8]

    ######################
    # Event data         #
    ######################

    def _briefdata( self ):
        #NB: The brief data is loaded by the C++ reader for each event.
        b = self._brief
        if b is None:
            h = self._h
            if not h[0]:
                raise RuntimeError( 'No active event' )
            b = self._brief = self._mv[h[1]:h[1]+h[2]]
            self._hdr = _s_trkhdr.unpack_from( b, 0 )
        return b

    def _fulldata( self ):
        f = self._full
        if f is None:
            self._r.load_full_data()
            n, = _struct.unpack_from( '<q', self._buf, _FULLSIZE_OFFSET )
            off = self._h[3]
            f = self._full = self._mv[off:off+n]
        return f

    def _metaDataIdx( self ):
        self._briefdata()
        return self._hdr[1]

    def runNumber( self ):
        return self._h[5]

    def eventNumber( self ):
        return self._h[6]

    def currentEventVersion( self ):
        return self._h[11]

    def seed( self ):
        """Random seed used for the event"""
        self._briefdata()
        return self._hdr[0]

    def seedStr( self ):
        return str( self.seed() )

    def eventStorageMode( self ):
        """Storage mode of the event ("FULL", "REDUCED" or "MINIMAL")"""
        self._briefdata()
        return _modes[self._hdr[3]]

    eventStorageModeStr = eventStorageMode

    def eventCheckSum( self ):
        return self._h[10]

    def verifyEventDataIntegrity( self ):
        """Recalculate the checksum of the event and compare with the one stored"""
        return self._r.verify_integrity()

    def nTracks( self ):
        self._briefdata()
        return self._hdr[2]

    def _loadTracks( self ):
        data = self._briefdata()
        pos = _SIZE_TRKHDR
        tracks = []
        dstructs = _daughter_structs
        for it in range( self._hdr[2] ):
            v = _trk_unpack( data, pos )
            nd = v[6]
            pos += _SIZE_TRK
            if nd:
                s = dstructs.get( nd )
                if s is None:
                    s = dstructs[nd] = _struct.Struct( '<%ii'%nd )
                daughters = s.unpack_from( data, pos )
                pos += 4 * nd
            else:
                daughters = ()
            tracks.append( Track( self, it, v, daughters ) )
        self._tracks = tracks
        self._segdatastart = pos#segment data follows the track data
        self._segoffsets = None
        self._contiguous = all( t._v[0] == i + 1 for i, t in enumerate( tracks ) )
        return tracks

    def _segOffsets( self ):
        #Offsets of the first segment of each track (found by only looking at the
        #flags of each segment, which indicate if extra data follows):
        offsets = self._segoffsets
        if offsets is None:
            data, pos = self._brief, self._segdatastart
            offsets = []
            for trk in self._tracks:
                offsets.append( pos )
                for _ in range( trk._v[5] ):
                    #Test bit 0x20000000 of the (little endian) flags at offset 16:
                    pos += ( _SIZE_SEG + _SIZE_SEG_EXTRA if data[ pos + 19 ] & 0x20
                             else _SIZE_SEG )
                pos += _SIZE_SEG_EXTRA#extra info for the last segment
            if pos != len( data ):
                raise RuntimeError( 'Inconsistent track data in Griff file' )
            self._segoffsets = offsets
        return offsets

    def _firstSegValues( self, trk ):
        return _seg_unpack( self._brief, self._segOffsets()[trk._it] )

    def _loadSegments( self, trk ):
        #NB: Most tracks have very few segments, so per-track overhead matters.
        it, data = trk._it, self._brief
        pos = ( self._segoffsets or self._segOffsets() )[it]
        n = trk._v[5]
        if n == 1:
            return [ Segment( self, it, _seg_unpack( data, pos ), 0 ) ]
        segs = []
        for i in range( n ):
            v = _seg_unpack( data, pos )
            segs.append( Segment( self, it, v, i ) )
            pos += ( _SIZE_SEG + _SIZE_SEG_EXTRA ) if v[2] & 0x20000000 else _SIZE_SEG
        return segs

    def _trks( self ):
        t = self._tracks
        return t if t is not None else self._loadTracks()

    def _loadSteps( self, seg ):
        raw = seg._v[3]
        if raw < 0:
            return ()#MINIMAL mode
        data = self._full
        if data is None:
            data = self._fulldata()
        n = _u32_unpack( data, raw + 4 )[0]
        it, iseg = seg._it, seg._i
        pos = raw + _SIZE_STEPHDR
        if n == 1:
            return ( Step( self, it, iseg, _step_unpack( data, pos ), 0 ), )
        return [ Step( self, it, iseg, _step_unpack( data, p ), k )
                 for k, p in enumerate( range( pos, pos + _SIZE_STEP * n, _SIZE_STEP ) ) ]

    def getTrack( self, i ):
        return self._trks()[i]

    def hasTrackID( self, trkid ):
        t = self._trks()
        if not t or trkid < 1:
            return False
        if not self._contiguous:
            raise RuntimeError( 'hasTrackID not implemented yet for all filtered events' )
        return trkid <= len( t )

    def getTrackByID( self, trkid ):
        t = self._trks()
        if not self._contiguous:
            raise RuntimeError( 'getTrackByID not implemented yet for all filtered events' )
        return t[trkid-1]

    def nPrimaryTracks( self ):
        return sum( 1 for _ in self.primaryTracks )

    @property
    def tracks( self ):
        """Iterate over all tracks in the event"""
        return iter( self._trks() )

    @property
    def primaryTracks( self ):
        """Iterate over the primary tracks in the event"""
        for t in self._trks():
            if t._v[4] != 0:
                break#all primary tracks are at the beginning
            yield t

    def setupChanged( self ):
        """True if setup() gives a different setup than in the previous call"""
        mdidx = self._metaDataIdx()
        ls = self._lastSetup
        if ls is None:
            return True
        if ls[0] == self._h[7]:
            return ls[1] != mdidx
        if not self._allowSetupChange:
            return False
        if self._metaDataMap( mdidx ) != ls[2]._allData():
            return True
        self._lastSetup = ( self._h[7], mdidx, ls[2] )
        return False

    def setup( self ):
        """Setup of the current event"""
        if not self.setupChanged():
            return self._lastSetup[2]
        mdidx = self._metaDataIdx()
        s = Setup( self._metaDataMap( mdidx ) )
        self._lastSetup = ( self._h[7], mdidx, s )
        return s

    def allowSetupChange( self ):
        """Allow events from different setups (by default an error is raised)"""
        self._allowSetupChange = True
