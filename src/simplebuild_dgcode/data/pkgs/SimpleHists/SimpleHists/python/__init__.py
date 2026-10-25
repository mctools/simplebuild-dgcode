__doc__='python module for package SimpleHists'
__all__=[
    'Hist1D',
    'Hist2D',
    'HistBase',
    'HistCollection',
    'HistCounts',
    'deserialise',
    'histTypeOfData',
]

###############################################################################
# 1) The histogram classes, implemented on top of the compiled C++ classes in
#    the _impl module (which provides the class _impl.Hist holding any
#    histogram, and _impl.Collection for loading and saving .shist files).

from . import _impl
from os import environ as _environ
import sys as _sys

def _to_bytes( data ):
    if isinstance( data, str ):
        data = data.encode( 'latin-1' )
    return bytes( data )

def _wrap( h ):
    """Python histogram object for the given _impl.Hist object"""
    cls = _histclasses[h.hist_type()]
    o = cls.__new__( cls )
    HistBase.__init__( o, h )
    return o

def _cppobj( h ):
    if not isinstance( h, HistBase ):
        raise TypeError( 'Expected a histogram object, got %s'%type( h ).__name__ )
    return h._h

class HistBase:
    """Base class of all histograms"""

    __slots__ = ( '__weakref__', '_h' )

    def __init__( self, h ):
        object.__setattr__( self, '_h', h )

    def getTitle( self ):
        return self._h.get_title()
    def getXLabel( self ):
        return self._h.get_xlabel()
    def getYLabel( self ):
        return self._h.get_ylabel()
    def getComment( self ):
        return self._h.get_comment()
    def setTitle( self, s ):
        self._h.set_title( s )
    def setXLabel( self, s ):
        self._h.set_xlabel( s )
    def setYLabel( self, s ):
        self._h.set_ylabel( s )
    def setComment( self, s ):
        self._h.set_comment( s )
    def histType( self ):
        return self._h.hist_type()
    def getIntegral( self ):
        return self._h.integral()
    def empty( self ):
        return self._h.empty()
    def dimension( self ):
        return self._h.dimension()
    def serialise( self ):
        return bytes.fromhex( self._h.serialise_hex() )
    def dump( self, contents = False, prefix = '' ):
        _sys.stdout.flush()
        self._h.dump( bool( contents ), prefix )
    def mergeCompatible( self, other ):
        return self._h.merge_compatible( _cppobj( other ) )
    def merge( self, other ):
        self._h.merge( _cppobj( other ) )
    def isSimilar( self, other ):
        return self._h.is_similar( _cppobj( other ) )
    def scale( self, f ):
        self._h.scale( f )
    def norm( self ):
        self._h.norm()
    def clone( self ):
        return _wrap( self._h.clone() )
    def reset( self ):
        self._h.reset()

    title = property( getTitle, setTitle )
    xlabel = property( getXLabel, setXLabel )
    ylabel = property( getYLabel, setYLabel )
    comment = property( getComment, setComment )
    integral = property( getIntegral )

    def __reduce__( self ):
        #pickle support:
        return ( deserialise, ( self.serialise(), ) )

def histTypeOfData( serialised_data ):
    """Type of histogram in serialised data (1: Hist1D, 2: Hist2D, 3: HistCounts)"""
    return chr( _impl.hist_type_of_data( _to_bytes( serialised_data ) ) )

def deserialise( serialised_data ):
    """Create histogram from serialised data"""
    return _wrap( _impl.Hist.deserialise( _to_bytes( serialised_data ) ) )

deserialiseAndManage = deserialise

def _check_serialised( data, histtype ):
    data = _to_bytes( data )
    if _impl.hist_type_of_data( data ) != histtype:
        raise ValueError( 'Serialised data is not of the right histogram type' )
    return data

class Hist1D( HistBase ):
    """One dimensional histogram. Create with Hist1D(nbins,xmin,xmax),
    Hist1D(title,nbins,xmin,xmax) or Hist1D(serialised_data)"""

    __slots__ = ()

    def __init__( self, *args, **kwargs ):
        if len( args ) == 1 and not kwargs or set( kwargs ) == { 'serialised_data' }:
            data = args[0] if args else kwargs['serialised_data']
            h = _impl.Hist.deserialise( _check_serialised( data, 0x01 ) )
        else:
            if args and isinstance( args[0], str ):
                kwargs['title'], args = args[0], args[1:]
            names = ( 'nbins', 'xmin', 'xmax' )
            vals = dict( zip( names, args ) )
            vals.update( kwargs )
            title = vals.pop( 'title', '' )
            if set( vals ) != set( names ):
                raise TypeError( 'Hist1D: invalid arguments' )
            h = _impl.Hist.new1d( title, vals['nbins'], vals['xmin'], vals['xmax'] )
        HistBase.__init__( self, h )

    def getNBins( self ):
        return self._h.h1_nbins()
    def getBinContent( self, i ):
        return self._h.h1_bincontent( i )
    def getBinError( self, i ):
        return self._h.h1_binerror( i )
    def getBinCenter( self, i ):
        return self._h.h1_bincenter( i )
    def getBinLower( self, i ):
        return self._h.h1_binlower( i )
    def getBinUpper( self, i ):
        return self._h.h1_binupper( i )
    def getBinWidth( self ):
        return self._h.h1_binwidth()
    def getMaxContent( self ):
        return self._h.h1_maxcontent()
    def getUnderflow( self ):
        return self._h.h1_underflow()
    def getOverflow( self ):
        return self._h.h1_overflow()
    def getMinFilled( self ):
        return self._h.h1_minfilled()
    def getMaxFilled( self ):
        return self._h.h1_maxfilled()
    def getXMin( self ):
        return self._h.h1_xmin()
    def getXMax( self ):
        return self._h.h1_xmax()
    def getMean( self ):
        return self._h.h1_mean()
    def getRMS( self ):
        return self._h.h1_rms()
    def getRMSSquared( self ):
        return self._h.h1_rms2()
    def valueToBin( self, v ):
        return self._h.h1_valuetobin( v )
    def fillN( self, n, v ):
        self._h.h1_filln( n, v )
    def _rawfill( self, v, w = None ):
        if w is None:
            self._h.h1_fill( v )
        else:
            self._h.h1_fillw( v, w )
    def _rawfillFromBuffer( self, v, w = None ):
        if w is None:
            self._h.h1_fillmany( v )
        else:
            self._h.h1_fillmanyw( v, w )
    def _rawContents( self ):
        return self._h.h1_contents()
    def _rawErrorsSquared( self ):
        return self._h.h1_errorssquared()
    def setErrorsByContent( self ):
        self._h.h1_seterrorsbycontent()
    def rebin( self, n ):
        self._h.h1_rebin( n )
    def canRebin( self, n ):
        return self._h.h1_canrebin( n )
    def resetAndRebin( self, n, xmin, xmax ):
        self._h.h1_resetandrebin( n, xmin, xmax )
    def getPercentileBin( self, percentile = 0.5 ):
        return self._h.h1_percentilebin( percentile )
    def getBinSum( self, bin1, bin2 ):
        return self._h.h1_binsum( bin1, bin2 )

    nbins = property( getNBins )
    binwidth = property( getBinWidth )
    underflow = property( getUnderflow )
    overflow = property( getOverflow )
    minfilled = property( getMinFilled )
    maxfilled = property( getMaxFilled )
    xmin = property( getXMin )
    xmax = property( getXMax )
    mean = property( getMean )
    rms = property( getRMS )
    rms2 = property( getRMSSquared )
    maxcontent = property( getMaxContent )

class Hist2D( HistBase ):
    """Two dimensional histogram. Create with
    Hist2D(nbinsx,xmin,xmax,nbinsy,ymin,ymax), Hist2D(title,nbinsx,...) or
    Hist2D(serialised_data)"""

    __slots__ = ()

    def __init__( self, *args, **kwargs ):
        if len( args ) == 1 and not kwargs or set( kwargs ) == { 'serialised_data' }:
            data = args[0] if args else kwargs['serialised_data']
            h = _impl.Hist.deserialise( _check_serialised( data, 0x02 ) )
        else:
            if args and isinstance( args[0], str ):
                kwargs['title'], args = args[0], args[1:]
            names = ( 'nbinsx', 'xmin', 'xmax', 'nbinsy', 'ymin', 'ymax' )
            vals = dict( zip( names, args ) )
            vals.update( kwargs )
            title = vals.pop( 'title', '' )
            if set( vals ) != set( names ):
                raise TypeError( 'Hist2D: invalid arguments' )
            h = _impl.Hist.new2d( title, *[ vals[n] for n in names ] )
        HistBase.__init__( self, h )

    def getNBinsX( self ):
        return self._h.h2_nbinsx()
    def getNBinsY( self ):
        return self._h.h2_nbinsy()
    def getBinContent( self, ix, iy ):
        return self._h.h2_bincontent( ix, iy )
    def getBinCenterX( self, i ):
        return self._h.h2_bincenterx( i )
    def getBinLowerX( self, i ):
        return self._h.h2_binlowerx( i )
    def getBinUpperX( self, i ):
        return self._h.h2_binupperx( i )
    def getBinCenterY( self, i ):
        return self._h.h2_bincentery( i )
    def getBinLowerY( self, i ):
        return self._h.h2_binlowery( i )
    def getBinUpperY( self, i ):
        return self._h.h2_binuppery( i )
    def getBinWidthX( self ):
        return self._h.h2_binwidthx()
    def getBinWidthY( self ):
        return self._h.h2_binwidthy()
    def getUnderflowX( self ):
        return self._h.h2_underflowx()
    def getOverflowX( self ):
        return self._h.h2_overflowx()
    def getUnderflowY( self ):
        return self._h.h2_underflowy()
    def getOverflowY( self ):
        return self._h.h2_overflowy()
    def getMinFilledX( self ):
        return self._h.h2_minfilledx()
    def getMaxFilledX( self ):
        return self._h.h2_maxfilledx()
    def getMinFilledY( self ):
        return self._h.h2_minfilledy()
    def getMaxFilledY( self ):
        return self._h.h2_maxfilledy()
    def getXMin( self ):
        return self._h.h2_xmin()
    def getXMax( self ):
        return self._h.h2_xmax()
    def getYMin( self ):
        return self._h.h2_ymin()
    def getYMax( self ):
        return self._h.h2_ymax()
    def getMeanX( self ):
        return self._h.h2_meanx()
    def getRMSX( self ):
        return self._h.h2_rmsx()
    def getRMSSquaredX( self ):
        return self._h.h2_rms2x()
    def getMeanY( self ):
        return self._h.h2_meany()
    def getRMSY( self ):
        return self._h.h2_rmsy()
    def getRMSSquaredY( self ):
        return self._h.h2_rms2y()
    def getCovariance( self ):
        return self._h.h2_covariance()
    def getCorrelation( self ):
        return self._h.h2_correlation()
    def valueToBinX( self, v ):
        return self._h.h2_valuetobinx( v )
    def valueToBinY( self, v ):
        return self._h.h2_valuetobiny( v )
    def _rawfill( self, x, y, w = None ):
        if w is None:
            self._h.h2_fill( x, y )
        else:
            self._h.h2_fillw( x, y, w )
    def _rawfillFromBuffer( self, x, y, w = None ):
        if w is None:
            self._h.h2_fillmany( x, y )
        else:
            self._h.h2_fillmanyw( x, y, w )
    def _rawContents( self ):
        return self._h.h2_contents()

    binwidthx = property( getBinWidthX )
    binwidthy = property( getBinWidthY )
    nbinsx = property( getNBinsX )
    nbinsy = property( getNBinsY )
    underflowx = property( getUnderflowX )
    overflowx = property( getOverflowX )
    underflowy = property( getUnderflowY )
    overflowy = property( getOverflowY )
    minfilledx = property( getMinFilledX )
    maxfilledx = property( getMaxFilledX )
    minfilledy = property( getMinFilledY )
    maxfilledy = property( getMaxFilledY )
    xmin = property( getXMin )
    xmax = property( getXMax )
    ymin = property( getYMin )
    ymax = property( getYMax )
    meanx = property( getMeanX )
    rmsx = property( getRMSX )
    rms2x = property( getRMSSquaredX )
    meany = property( getMeanY )
    rmsy = property( getRMSY )
    rms2y = property( getRMSSquaredY )
    covariance = property( getCovariance )
    covxy = property( getCovariance )
    correlation = property( getCorrelation )
    corxy = property( getCorrelation )

class Counter:
    """Counter in a HistCounts histogram"""

    __slots__ = ( '_hc', '_label' )

    def __init__( self, hc, label ):
        self._hc, self._label = hc, label

    def getValue( self ):
        return self._hc._h.hc_value( self._label )
    __call__ = getValue
    def getError( self ):
        return self._hc._h.hc_error( self._label )
    def getErrorSquared( self ):
        return self._hc._h.hc_errorsquared( self._label )
    def getLabel( self ):
        return self._label
    def getDisplayLabel( self ):
        return self._hc._h.hc_displaylabel( self._label )
    def setDisplayLabel( self, s ):
        self._hc._h.hc_setdisplaylabel( self._label, s )
    def getComment( self ):
        return self._hc._h.hc_comment( self._label )
    def setComment( self, s ):
        self._hc._h.hc_setcomment( self._label, s )
    def __iadd__( self, other ):
        if isinstance( other, Counter ):
            self._hc._h.hc_addcounter_from( self._label, other._hc._h, other._label )
        else:
            self._hc._h.hc_add( self._label, other )
        return self

    value = property( getValue )
    error = property( getError )
    errorsquared = property( getErrorSquared )
    label = property( getLabel )
    displaylabel = property( getDisplayLabel, setDisplayLabel )
    comment = property( getComment, setComment )

class HistCounts( HistBase ):
    """Histogram with named counters. Create with HistCounts(),
    HistCounts(title) or HistCounts(serialised_data)"""

    __slots__ = ()

    def __init__( self, arg = None, *, title = None, serialised_data = None ):
        if title is not None:
            arg = title
        elif serialised_data is not None:
            arg = _to_bytes( serialised_data )
        if isinstance( arg, ( bytes, bytearray, memoryview ) ):
            h = _impl.Hist.deserialise( _check_serialised( arg, 0x03 ) )
        else:
            h = _impl.Hist.newcounts( arg or '' )
        HistBase.__init__( self, h )

    def addCounter( self, label, display_text = '' ):
        return Counter( self, self._h.hc_addcounter( label, display_text ) )
    def getMaxContent( self ):
        return self._h.hc_maxcontent()
    def setErrorsByContent( self ):
        self._h.hc_seterrorsbycontent()
    def sortByLabels( self ):
        self._h.hc_sortbylabels()
    def sortByDisplayLabels( self ):
        self._h.hc_sortbydisplaylabels()
    def getCounters( self ):
        labels = self._h.hc_labels()
        return [ Counter( self, label ) for label in labels.split( '\n' ) ] if labels else []
    def getCounter( self, label ):
        if not self._h.hc_hascounter( label ):
            raise KeyError( 'No counter with label "%s"'%label )
        return Counter( self, label )
    def hasCounter( self, label ):
        return self._h.hc_hascounter( label )
    def nCounters( self ):
        return self._h.hc_ncounters()

    ncounters = property( nCounters )
    maxcontent = property( getMaxContent )
    counters = property( getCounters )

_histclasses = { 0x01 : Hist1D, 0x02 : Hist2D, 0x03 : HistCounts }

class HistCollection:
    """Collection of histograms with unique keys, which can be saved to and
    loaded from .shist files."""

    def __init__( self, filename = None ):
        self._hists = {}
        if filename is not None:
            if hasattr( filename, '__fspath__' ):
                filename = filename.__fspath__()
            c = _impl.Collection.load( filename )
            keys = c.keys()
            for k in ( keys.split( '\n' ) if keys else [] ):
                self._hists[k] = _wrap( c.take( k ) )

    def _add( self, h, key ):
        _impl.check_key( key )
        if key in self._hists:
            _sys.stdout.flush()
            print( 'HistCollection ERROR duplicate key "%s"'%key, flush = True )
            raise RuntimeError( 'HistCollection duplicate key' )
        self._hists[key] = h
        return h

    def book1D( self, *args ):
        """book1D(nbins,xmin,xmax,key) or book1D(title,nbins,xmin,xmax,key)"""
        if len( args ) not in ( 4, 5 ):
            raise TypeError( 'book1D: invalid arguments' )
        return self._add( Hist1D( *args[:-1] ), args[-1] )

    def book2D( self, *args ):
        """book2D(nbinsx,xmin,xmax,nbinsy,ymin,ymax,key) or book2D(title,nbinsx,...,key)"""
        if len( args ) not in ( 7, 8 ):
            raise TypeError( 'book2D: invalid arguments' )
        return self._add( Hist2D( *args[:-1] ), args[-1] )

    def bookCounts( self, *args ):
        """bookCounts(key) or bookCounts(title,key)"""
        if len( args ) not in ( 1, 2 ):
            raise TypeError( 'bookCounts: invalid arguments' )
        return self._add( HistCounts( args[0] if len( args ) == 2 else '' ), args[-1] )

    def add( self, h, key ):
        """Add histogram with the given key"""
        _cppobj( h )
        self._add( h, key )

    def hasKey( self, key ):
        return key in self._hists

    def getKey( self, h ):
        for k, o in self._hists.items():
            if o._h.same( _cppobj( h ) ):
                return k
        raise KeyError( 'Histogram not in collection' )

    def hist( self, key ):
        h = self._hists.get( key )
        if h is None:
            _sys.stdout.flush()
            print( 'HistCollection::hist ERROR unknown key "%s"'%key, flush = True )
            raise RuntimeError( 'HistCollection::hist unknown key' )
        return h

    def remove( self, key ):
        """Remove histogram from collection and return it"""
        return self._hists.pop( key )

    removeAndManage = remove

    def saveToFile( self, filename, allowOverwrite = False ):
        if hasattr( filename, '__fspath__' ):
            filename = filename.__fspath__()
        c = _impl.Collection.create()
        for k in sorted( self._hists ):
            c.add_clone( self._hists[k]._h, k )
        _sys.stdout.flush()
        c.save( filename, bool( allowOverwrite ) )

    def getKeys( self ):
        return sorted( self._hists )

    keys = property( getKeys )

    def merge( self, other ):
        """Merge with another collection (or a collection in a file), which must
        have identical keys and compatible histograms"""
        if not isinstance( other, HistCollection ):
            other = HistCollection( other )
        e = 'HistCollection attempted merge with incompatible collection'
        if self.getKeys() != other.getKeys():
            raise RuntimeError( e )
        for k, h in self._hists.items():
            if not h.mergeCompatible( other._hists[k] ):
                raise RuntimeError( e )
        for k, h in self._hists.items():
            h.merge( other._hists[k] )

    def isSimilar( self, other ):
        if self.getKeys() != other.getKeys():
            return False
        return all( h.isSimilar( other._hists[k] ) for k, h in self._hists.items() )

###############################################################################
# 2) Detect presence of numpy and if present, we add methods for getting
#    contents/errors as numpy arrays as well as a .histogram[2d] method which
#    returns contents in a format similar to that returned by
#    numpy.histogram(..) and numpy.histogram2d(..). We also enable efficient
#    filling with numpy arrays.

try:
    import numpy
except ImportError:
    numpy=None

if numpy:
    import SimpleHists._numpyutils as nu
    Hist1D.bar_args = nu.h1d_bar_args
    Hist1D.errorbar_args = nu.h1d_errorbar_args
    Hist1D.contents = nu.h1d_contents
    Hist1D.errors = nu.h1d_errors
    Hist1D.binedges = nu.h1d_binedges
    Hist1D.bincenters = nu.h1d_bincenters
    Hist1D.curve = nu.h1d_curve
    Hist1D.errorband = nu.h1d_errorband
    Hist1D.histogram = nu.h1d_histogram
    Hist1D.fill = nu.h1d_fill
    Hist2D.contents = nu.h2d_contents
    Hist2D.xedges = nu.h2d_xedges
    Hist2D.yedges = nu.h2d_yedges
    Hist2D.extent = nu.h2d_extent
    Hist2D.imshow_args = nu.h2d_imshow_args
    Hist2D.bar3d_args = nu.h2d_bar3d_args
    Hist2D.histogram2d = nu.h2d_histogram2d
    Hist2D.fill = nu.h2d_fill
    HistCounts.bar_args = nu.hcounts_bar_args
    HistCounts.errorbar_args = nu.hcounts_errorbar_args
else:
    Hist1D.fill=Hist1D._rawfill
    Hist2D.fill=Hist2D._rawfill

################################################################################
# 3) Detect presence of matplotlib and if present, we add methods for quickly
#    plotting the histograms in interactive viewers.

matplotlib=None
if numpy:
    try:
        import matplotlib
    except ImportError:
        pass



def is_osx_catalina_or_later():
    import platform
    if platform.system()!='Darwin':
        return False
    return int(platform.release().split('.')[0])>=19

_lacks_display_var = not _environ.get('DISPLAY',None)
_catalina_mode = is_osx_catalina_or_later()
_has_agg_backend = (matplotlib and matplotlib.get_backend().lower()=='agg')
if (not _has_agg_backend) and _lacks_display_var and _catalina_mode:
    #insert dummy value to prevent matplotlib from switching to the agg backend:
    _environ['DISPLAY']=':0'
    _lacks_display_var=False
_noninteractive = _has_agg_backend or _lacks_display_var
if 'SIMPLEHISTS_SILENT_PLOTFAIL' in _environ:
    _noninteractive = True

if (not _noninteractive) and matplotlib:
    from SimpleHists import plotutils
    Hist1D.plot = plotutils.plot1d
    Hist1D.overlay = plotutils.plot1d_overlay
    Hist2D.plot = plotutils.plot2d
    Hist2D.plot_lego = plotutils.plot2d_lego
    HistCounts.plot = plotutils.plotcounts
else:
    def no_plot(self,*args,**kwargs):
        if 'SIMPLEHISTS_SILENT_PLOTFAIL' in _environ:
            return
        if _noninteractive:
            reason=('running in non-interactive mode (agg backend'
                    ' selected or empty DISPLAY variable can cause this)')
        else:
            reason='please install numpy and matplotlib'
        raise SystemExit(f"ERROR: Plotting not available ({reason})")
    Hist1D.plot = no_plot
    Hist1D.overlay = no_plot
    Hist2D.plot = no_plot
    Hist2D.plot_lego = no_plot
    HistCounts.plot = no_plot

###############################################################################
# 4) Make histograms accessible as HistCollection properties so they can be
#    easily accessed via tab completion in ipython. Note that this only works
#    nicely for keys of the form (alpha)+(alpha/digit/_)*

class _histgetter:
    def __init__(self,hc):
        self._hc=hc
    def __dir__(self):
        return dir(self.__class__)+self._hc.keys
    def __getattr__(self,k):
        if k[0]=='_':
            raise AttributeError(k)
        return self._hc.hist(k)
    def __iter__(self):
        for k in self._hc.keys:
            yield self._hc.hist(k)

HistCollection.hist_getter = property(lambda self: _histgetter(self))

###############################################################################
# 5) Make counts accessible as HistCounts properties so they can be easily
#    accessed via tab completion in ipython.

class _countgetter:
    def __init__(self,hc):
        self._hc=hc
    def __dir__(self):
        res=dir(self.__class__)
        for c in self._hc.counters:
            res+=[c.label]
        return res
    def __getattr__(self,k):
        if k[0]=='_':
            raise AttributeError(k)
        return self._hc.getCounter(k)

HistCounts.c = property(lambda self: _countgetter(self))
