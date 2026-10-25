"""utilities for easy and efficient analysis of .griff data files containing results from Geant4 simulations"""
__author__='thomas.kittelmann@ess.eu'

#Pure Python implementation of the iterators and filters of the GriffAnaUtils
#package (see the C++ classes in GriffAnaUtils/libinc/*.hh). Filters are
#created with the create(..) methods and added to iterators with addFilter(..)
#(which returns the filter, for convenience). Each iterator yields the
#selected objects in the current event of the GriffDataReader.

import struct as _struct

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

_DBL_MAX = 1.7976931348623157e308

def _to_float32( x ):
    #Thresholds are stored as single precision floats in the C++ classes:
    return _struct.unpack( '<f', _struct.pack( '<f', x ) )[0]

class IFilter:
    """Base class of all filters"""
    def __init__( self ):
        self._negated = False
        self._enabled = True
    @classmethod
    def create( cls, *args ):
        return cls( *args )
    def setNegated( self, n = True ):
        self._negated = bool( n )
    def toggleNegation( self ):
        self._negated = not self._negated
    def negated( self ):
        return self._negated
    def enabled( self ):
        return self._enabled
    def setEnabled( self, b = True ):
        old, self._enabled = self._enabled, bool( b )
        return old
    def setDisabled( self, b = True ):
        old, self._enabled = not self._enabled, not b
        return old
    #Reference counting is not needed in Python (kept for compatibility):
    def ref( self ):
        pass
    def unref( self ):
        pass
    def refCount( self ):
        return 1

class ITrackFilter( IFilter ):
    def filter( self, trk ):
        raise NotImplementedError

class ISegmentFilter( IFilter ):
    def filter( self, segment ):
        raise NotImplementedError
    def associatedTrackFilter( self ):
        return None

class IStepFilter( IFilter ):
    def filter( self, step ):
        raise NotImplementedError
    def associatedSegmentFilter( self ):
        return None
    def associatedTrackFilter( self ):
        return None

def _passes( filters, o ):
    for f in filters:
        if f._enabled and f.filter( o ) == f._negated:
            return False
    return True

################################################################################
# Track filters                                                                #
################################################################################

class TrackFilter_PDGCode( ITrackFilter ):
    """Select tracks with given PDG codes (by default the signs must match,
    call setUnsigned() to ignore signs)"""
    def __init__( self, *codes ):
        super().__init__()
        self._unsigned = False
        self._positive, self._negative, self._abs = set(), set(), set()
        self.addCodes( *codes )
    def addCode( self, c ):
        c = int( c )
        ( self._negative if c < 0 else self._positive ).add( c )
        self._abs.add( abs( c ) )
        return self
    def addCodes( self, *codes ):
        for c in codes:
            self.addCode( c )
        return self
    def setUnsigned( self ):
        self._unsigned = True
        return self
    def setSigned( self ):
        self._unsigned = False
        return self
    def isUnsigned( self ):
        return self._unsigned
    def isSigned( self ):
        return not self._unsigned
    def filter( self, trk ):
        c = trk.pdgCode()
        if self._unsigned:
            return abs( c ) in self._abs
        return c in ( self._negative if c < 0 else self._positive )

class TrackFilter_Descendant( ITrackFilter ):
    """Select descendants of a given track (setAncestor must be called in each
    event)"""
    def __init__( self ):
        super().__init__()
        self._ancestor = None
        self._includeAncestor = True
        self._eventTracks = None
    def setAncestor( self, trk, includeAncestor = True ):
        self._ancestor = trk
        self._includeAncestor = bool( includeAncestor )
        self._eventTracks = trk.getDataReader()._trks()#identifies the event
    def filter( self, trk ):
        a = self._ancestor
        if a is None or a.getDataReader()._tracks is not self._eventTracks:
            raise RuntimeError( 'TrackFilter_Descendant ERROR: setAncestor must be invoked each event before filter is used!!' )
        aid = a.trackID()
        if trk.trackID() == aid:
            return self._includeAncestor
        while True:
            pid = trk.parentID()
            if pid == 0:
                return False
            if pid == aid:
                return True
            trk = trk.getParent()

class TrackFilter_Primary( ITrackFilter ):
    """Select primary tracks"""
    def filter( self, trk ):
        return trk.isPrimary()

class TrackFilter_Charged( ITrackFilter ):
    """Select charged tracks (optionally only negative or positive ones)"""
    def __init__( self ):
        super().__init__()
        self._allowNegative = True
        self._allowPositive = True
    def setNegativeOnly( self ):
        self._allowNegative, self._allowPositive = True, False
        return self
    def setPositiveOnly( self ):
        self._allowNegative, self._allowPositive = False, True
        return self
    def filter( self, trk ):
        q = trk.charge()
        if not q:
            return False
        return self._allowNegative if q < 0 else self._allowPositive

################################################################################
# Segment filters                                                              #
################################################################################

class SegmentFilter_Volume( ISegmentFilter ):
    """Select segments in a given volume (optionally also requiring given
    mother and grandmother volumes). Names are those of logical volumes,
    unless setPhysical() is called."""
    def __init__( self, volumeName, motherVolumeName = None, grandMotherVolumeName = None ):
        super().__init__()
        self._names = [ volumeName ]
        if motherVolumeName:
            self._names.append( motherVolumeName )
            if grandMotherVolumeName:
                self._names.append( grandMotherVolumeName )
        self._logical = True
    def setPhysical( self ):
        self._logical = False
        return self
    def setLogical( self ):
        self._logical = True
        return self
    def isPhysical( self ):
        return not self._logical
    def isLogical( self ):
        return self._logical
    def filter( self, segment ):
        name = segment.volumeName if self._logical else segment.physicalVolumeName
        names = self._names
        if names[0] != name( 0 ):
            return False
        if len( names ) == 1:
            return True
        if len( names ) > segment.volumeDepthStored():
            return False
        return all( names[i] == name( i ) for i in range( 1, len( names ) ) )

class SegmentFilter_EnergyDeposition( ISegmentFilter ):
    """Select segments with an energy deposition above a threshold"""
    def __init__( self, threshold ):
        super().__init__()
        self._threshold = _to_float32( threshold )
    def filter( self, segment ):
        return segment.eDep() > self._threshold

class SegmentFilter_EKin( ISegmentFilter ):
    """Select segments with kinetic energies in given ranges"""
    def __init__( self ):
        super().__init__()
        self._minStart, self._maxStart = -1.0, _DBL_MAX
        self._minEnd, self._maxEnd = -1.0, _DBL_MAX
    #NB: The C++ class has a bug in setMinStartEKin/setMaxStartEKin (setting
    #the end values), which is reproduced here for consistency:
    def setMinStartEKin( self, e ):
        self._minEnd = e
        return self
    def setMaxStartEKin( self, e ):
        self._maxEnd = e
        return self
    def setMinEndEKin( self, e ):
        self._minEnd = e
        return self
    def setMaxEndEKin( self, e ):
        self._maxEnd = e
        return self
    def filter( self, segment ):
        pre = segment.startEKin()
        if pre < self._minStart or pre > self._maxStart:
            return False
        post = segment.endEKin()
        return not ( post < self._minEnd or post > self._maxEnd )

################################################################################
# Step filters                                                                 #
################################################################################

class StepFilter_EnergyDeposition( IStepFilter ):
    """Select steps with an energy deposition above a threshold"""
    def __init__( self, threshold ):
        super().__init__()
        self._threshold = threshold
        self._segmFilter = None
    def filter( self, step ):
        return step.eDep() > self._threshold
    def associatedSegmentFilter( self ):
        if self._segmFilter is None:
            self._segmFilter = SegmentFilter_EnergyDeposition( self._threshold )
        return self._segmFilter

class StepFilter_EKin( IStepFilter ):
    """Select steps with kinetic energies in given ranges"""
    def __init__( self ):
        super().__init__()
        self._minPre, self._maxPre = -1.0, _DBL_MAX
        self._minPost, self._maxPost = -1.0, _DBL_MAX
    def setMinPreEKin( self, e ):
        self._minPre = e
        return self
    def setMaxPreEKin( self, e ):
        self._maxPre = e
        return self
    def setMinPostEKin( self, e ):
        self._minPost = e
        return self
    def setMaxPostEKin( self, e ):
        self._maxPost = e
        return self
    def filter( self, step ):
        pre = step.preEKin()
        if pre < self._minPre or pre > self._maxPre:
            return False
        post = step.postEKin()
        return not ( post < self._minPost or post > self._maxPost )

################################################################################
# Iterators                                                                    #
################################################################################

class TrackIterator:
    """Iterates over the tracks of the current event passing all filters"""
    def __init__( self, dr ):
        self._dr = dr
        self._trackFilters = []
    def addFilter( self, f ):
        if not isinstance( f, ITrackFilter ):
            raise TypeError( 'Only track filters can be added to a TrackIterator' )
        self._trackFilters.append( f )
        return f
    def __iter__( self ):
        if not self._dr.eventActive():
            return
        tf = self._trackFilters
        for trk in self._dr._trks():
            if not tf or _passes( tf, trk ):
                yield trk

class SegmentIterator:
    """Iterates over the segments of the current event passing all filters"""
    def __init__( self, dr ):
        self._trkIter = TrackIterator( dr )
        self._segmentFilters = []
    def addFilter( self, f ):
        if isinstance( f, ITrackFilter ):
            self._trkIter.addFilter( f )
        elif isinstance( f, ISegmentFilter ):
            self._segmentFilters.append( f )
            tf = f.associatedTrackFilter()
            if tf:
                self._trkIter.addFilter( tf )
        else:
            raise TypeError( 'Only track or segment filters can be added to a SegmentIterator' )
        return f
    def __iter__( self ):
        sf = self._segmentFilters
        for trk in self._trkIter:
            for seg in trk._segs:
                if not sf or _passes( sf, seg ):
                    yield seg

class StepIterator:
    """Iterates over the steps of the current event passing all filters"""
    def __init__( self, dr ):
        self._segIter = SegmentIterator( dr )
        self._stepFilters = []
    def addFilter( self, f ):
        if isinstance( f, IStepFilter ):
            self._stepFilters.append( f )
            sf, tf = f.associatedSegmentFilter(), f.associatedTrackFilter()
            if sf:
                self._segIter.addFilter( sf )
            if tf:
                self._segIter.addFilter( tf )
        else:
            self._segIter.addFilter( f )
        return f
    def __iter__( self ):
        stf = self._stepFilters
        for seg in self._segIter:
            for step in seg._steps():
                if not stf or _passes( stf, step ):
                    yield step
