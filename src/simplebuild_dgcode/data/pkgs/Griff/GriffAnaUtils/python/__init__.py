"""utilities for easy and efficient analysis of .griff data files containing results from Geant4 simulations"""
__author__='thomas.kittelmann@ess.eu'

#The iterators and filters of the GriffAnaUtils package (see the C++ classes in
#GriffAnaUtils/libinc/*.hh). Filters are created with the create(..) methods
#and added to iterators with addFilter(..) (which returns the filter, for
#convenience). Each iterator yields the selected objects in the current event
#of the GriffDataReader.
#
#The iterators and the provided filters are implemented in C++ (in the _impl
#module), and only the selected objects are created in Python. Custom filters
#can be implemented in Python by deriving from ITrackFilter, ISegmentFilter or
#IStepFilter and implementing the filter(..) method. They are applied (in
#Python) to the objects selected by the other filters.

import numpy as _np
from . import _impl

__all__ = [ 'IFilter',
            'ISegmentFilter',
            'IStepFilter',
            'ITrackFilter',
            'SegmentFilter_EKin',
            'SegmentFilter_EnergyDeposition',
            'SegmentFilter_Volume',
            'SegmentIterator',
            'StepFilter_EKin',
            'StepFilter_EnergyDeposition',
            'StepIterator',
            'TrackFilter_Charged',
            'TrackFilter_Descendant',
            'TrackFilter_PDGCode',
            'TrackFilter_Primary',
            'TrackIterator' ]

_TRACK, _SEGMENT, _STEP = 0, 1, 2

class IFilter:
    """Base class of all filters"""
    #Defaults (also for custom filters not calling IFilter.__init__):
    _cf = None#C++ filter (for the provided filters)
    _negated = False
    _enabled = True
    def __init__( self ):
        pass
    @classmethod
    def create( cls, *args ):
        return cls( *args )
    def setNegated( self, n = True ):
        if self._cf:
            self._cf.set_negated( bool( n ) )
        else:
            self._negated = bool( n )
    def toggleNegation( self ):
        self.setNegated( not self.negated() )
    def negated( self ):
        return self._cf.negated() if self._cf else self._negated
    def enabled( self ):
        return self._cf.enabled() if self._cf else self._enabled
    def setEnabled( self, b = True ):
        old = self.enabled()
        if self._cf:
            self._cf.set_enabled( bool( b ) )
        else:
            self._enabled = bool( b )
        return old
    def setDisabled( self, b = True ):
        return not self.setEnabled( not b )
    #Reference counting is not needed in Python (kept for compatibility):
    def ref( self ):
        pass
    def unref( self ):
        pass
    def refCount( self ):
        return 1

class ITrackFilter( IFilter ):
    _kind = _TRACK
    def filter( self, trk ):
        if not self._cf:
            raise NotImplementedError
        return self._cf.filter_track( trk._dr._rh, trk._it )

class ISegmentFilter( IFilter ):
    _kind = _SEGMENT
    def filter( self, segment ):
        if not self._cf:
            raise NotImplementedError
        return self._cf.filter_segment( segment._dr._rh, segment._it, segment._i )
    def associatedTrackFilter( self ):
        return None

class IStepFilter( IFilter ):
    _kind = _STEP
    def filter( self, step ):
        if not self._cf:
            raise NotImplementedError
        return self._cf.filter_step( step._dr._rh, step._it, step._is, step._i )
    def associatedSegmentFilter( self ):
        return None
    def associatedTrackFilter( self ):
        return None

################################################################################
# Track filters                                                                #
################################################################################

class TrackFilter_PDGCode( ITrackFilter ):
    """Select tracks with given PDG codes (by default the signs must match,
    call setUnsigned() to ignore signs)"""
    def __init__( self, *codes ):
        self._cf = _impl.track_filter_pdgcode()
        self.addCodes( *codes )
    def addCode( self, c ):
        self._cf.pdg_add_code( int( c ) )
        return self
    def addCodes( self, *codes ):
        for c in codes:
            self.addCode( c )
        return self
    def setUnsigned( self ):
        self._cf.pdg_set_unsigned( True )
        return self
    def setSigned( self ):
        self._cf.pdg_set_unsigned( False )
        return self
    def isUnsigned( self ):
        return self._cf.pdg_is_unsigned()
    def isSigned( self ):
        return not self.isUnsigned()

class TrackFilter_Descendant( ITrackFilter ):
    """Select descendants of a given track (setAncestor must be called in each
    event)"""
    def __init__( self ):
        self._cf = _impl.track_filter_descendant()
    def setAncestor( self, trk, includeAncestor = True ):
        self._cf.descendant_set_ancestor( trk._dr._rh, trk._it, bool( includeAncestor ) )

class TrackFilter_Primary( ITrackFilter ):
    """Select primary tracks"""
    def __init__( self ):
        self._cf = _impl.track_filter_primary()

class TrackFilter_Charged( ITrackFilter ):
    """Select charged tracks (optionally only negative or positive ones)"""
    def __init__( self ):
        self._cf = _impl.track_filter_charged()
    def setNegativeOnly( self ):
        self._cf.charged_negative_only()
        return self
    def setPositiveOnly( self ):
        self._cf.charged_positive_only()
        return self

################################################################################
# Segment filters                                                              #
################################################################################

class SegmentFilter_Volume( ISegmentFilter ):
    """Select segments in a given volume (optionally also requiring given
    mother and grandmother volumes). Names are those of logical volumes,
    unless setPhysical() is called."""
    def __init__( self, volumeName, motherVolumeName = None, grandMotherVolumeName = None ):
        if grandMotherVolumeName and not motherVolumeName:
            raise ValueError( 'grandMotherVolumeName requires motherVolumeName' )
        self._cf = _impl.segment_filter_volume( volumeName, motherVolumeName or '',
                                                grandMotherVolumeName or '' )
    def setPhysical( self ):
        self._cf.volume_set_physical( True )
        return self
    def setLogical( self ):
        self._cf.volume_set_physical( False )
        return self
    def isPhysical( self ):
        return self._cf.volume_is_physical()
    def isLogical( self ):
        return not self.isPhysical()

class SegmentFilter_EnergyDeposition( ISegmentFilter ):
    """Select segments with an energy deposition above a threshold"""
    def __init__( self, threshold ):
        self._cf = _impl.segment_filter_edep( threshold )

class SegmentFilter_EKin( ISegmentFilter ):
    """Select segments with kinetic energies in given ranges"""
    def __init__( self ):
        self._cf = _impl.segment_filter_ekin()
    def setMinStartEKin( self, e ):
        self._cf.ekin_set( 0, e )
        return self
    def setMaxStartEKin( self, e ):
        self._cf.ekin_set( 1, e )
        return self
    def setMinEndEKin( self, e ):
        self._cf.ekin_set( 2, e )
        return self
    def setMaxEndEKin( self, e ):
        self._cf.ekin_set( 3, e )
        return self

################################################################################
# Step filters                                                                 #
################################################################################

class StepFilter_EnergyDeposition( IStepFilter ):
    """Select steps with an energy deposition above a threshold (associated
    segment filters are handled internally by the iterators)"""
    def __init__( self, threshold ):
        self._cf = _impl.step_filter_edep( threshold )

class StepFilter_EKin( IStepFilter ):
    """Select steps with kinetic energies in given ranges"""
    def __init__( self ):
        self._cf = _impl.step_filter_ekin()
    def setMinPreEKin( self, e ):
        self._cf.ekin_set( 0, e )
        return self
    def setMaxPreEKin( self, e ):
        self._cf.ekin_set( 1, e )
        return self
    def setMinPostEKin( self, e ):
        self._cf.ekin_set( 2, e )
        return self
    def setMaxPostEKin( self, e ):
        self._cf.ekin_set( 3, e )
        return self

################################################################################
# Iterators                                                                    #
################################################################################

def _passes( filters, o ):
    for f in filters:
        if f.enabled() and f.filter( o ) == f.negated():
            return False
    return True

class _Iterator:
    _kind = None
    _what = None

    def __init__( self, dr ):
        self._dr = dr
        #Buffer filled by the C++ iterator for each event (with the number of
        #selected objects followed by their indices):
        self._buf = _np.empty( max( 1, int( dr._buffer_mb * 1024 * 1024 ) // 4 ), dtype = _np.int32 )
        self._it = _impl.Iterator( self._kind, dr._rh, self._buf )
        self._pyfilters = ( [], [], [] )#custom filters (per kind)

    def addFilter( self, f ):
        if not isinstance( f, IFilter ) or getattr( f, '_kind', None ) is None or f._kind > self._kind:
            raise TypeError( 'Only %s filters can be added to a %s'%( self._what, self.__class__.__name__ ) )
        if f._cf:
            self._it.add_filter( f._cf )
        else:
            self._pyfilters[f._kind].append( f )
            #Associated filters of custom filters:
            if f._kind == _STEP:
                for af in ( f.associatedSegmentFilter(), f.associatedTrackFilter() ):
                    if af:
                        self.addFilter( af )
            elif f._kind == _SEGMENT:
                af = f.associatedTrackFilter()
                if af:
                    self.addFilter( af )
        return f

    def _indices( self ):
        #Indices of the selected objects in the current event:
        if not self._dr.eventActive():
            return []
        self._it.fill()
        buf = self._buf
        return buf[1:1+int( buf[0] )*( self._kind + 1 )].tolist()

class TrackIterator( _Iterator ):
    """Iterates over the tracks of the current event passing all filters"""
    _kind = _TRACK
    _what = 'track'
    def __iter__( self ):
        idx = self._indices()
        trks = self._dr._trks()
        tf = self._pyfilters[_TRACK]
        for i in idx:
            trk = trks[i]
            if not tf or _passes( tf, trk ):
                yield trk

class SegmentIterator( _Iterator ):
    """Iterates over the segments of the current event passing all filters"""
    _kind = _SEGMENT
    _what = 'track or segment'
    def __iter__( self ):
        idx = self._indices()
        trks = self._dr._trks()
        tf, sf, _ = self._pyfilters
        itrk, segs = -1, None
        for k in range( 0, len( idx ), 2 ):
            if idx[k] != itrk:
                itrk = idx[k]
                trk = trks[itrk]
                segs = trk._sg() if ( not tf or _passes( tf, trk ) ) else None
            if segs is not None:
                seg = segs[idx[k+1]]
                if not sf or _passes( sf, seg ):
                    yield seg

class StepIterator( _Iterator ):
    """Iterates over the steps of the current event passing all filters"""
    _kind = _STEP
    _what = 'track, segment or step'
    def __iter__( self ):
        idx = self._indices()
        trks = self._dr._trks()
        tf, sf, stf = self._pyfilters
        itrk = iseg = -1
        segs = steps = None
        for k in range( 0, len( idx ), 3 ):
            if idx[k] != itrk:
                itrk, iseg = idx[k], -1
                trk = trks[itrk]
                segs = trk._sg() if ( not tf or _passes( tf, trk ) ) else None
            if segs is None:
                continue
            if idx[k+1] != iseg:
                iseg = idx[k+1]
                seg = segs[iseg]
                steps = seg._steps() if ( not sf or _passes( sf, seg ) ) else None
            if steps is not None:
                step = steps[idx[k+2]]
                if not stf or _passes( stf, step ):
                    yield step
