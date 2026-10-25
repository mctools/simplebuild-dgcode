"""Python module for reading .mesh3d files (as produced for instance by
G4HeatMap), and for merging them."""

from . import _impl

__all__ = [ 'Mesh3D', 'merge_files' ]

class Mesh3D:
    """Contents of a .mesh3d file. The cell contents are available as a numpy
    array (the data attribute) with shape (nx,ny,nz)."""

    def __init__( self, filename ):
        f = _impl.MeshFile( str( filename ) )
        self._name = f.name()
        self._comments = f.comments()
        self._cellunits = f.cellunits()
        keys = f.statkeys()
        self._stats = dict( zip( keys.split( '\n' ) if keys else [],
                                 ( f.statval( i ) for i in range( f.nstats() ) ) ) )
        self._cellinfo = [ f.cells( i ) for i in range( 3 ) ]
        self._data = f.data().reshape( tuple( c[0] for c in self._cellinfo ) )

    @property
    def name( self ):
        return self._name

    @property
    def comments( self ):
        return self._comments

    @property
    def cellunits( self ):
        return self._cellunits

    @property
    def data( self ):
        return self._data

    @property
    def cellinfo( self ):
        """List of (ncells,lower,upper) for each of the three axes"""
        return list( self._cellinfo )

    @property
    def stats( self ):
        return dict( self._stats )

    def print_summary( self ):
        print( 'Mesh3D:' )
        print( '  Name       : %s'%( self._name or '<none>' ) )
        print( '  Comments   : %s'%( self._comments or '<none>' ) )
        print( '  Cell units : %s'%( self._cellunits or '<none>' ) )
        print( '  Cells      : [%s]'%', '.join( '(%s, %s, %s)'%c for c in self._cellinfo ) )
        print( '  Stats      : {%s}'%', '.join( "'%s': %s"%( k, self._stats[k] ) for k in sorted( self._stats ) ) )

    def dump_cells( self, include_empty ):
        nx, ny, nz = self._data.shape
        d = self._data
        for ix in range( nx ):
            for iy in range( ny ):
                for iz in range( nz ):
                    v = float( d[ix,iy,iz] )
                    if v or include_empty:
                        print( '  cell[%i,%i,%i] = %g'%( ix, iy, iz, v ) )

def merge_files( output_file, input_files ):
    """Merge the input files (a list of at least two files) into the output file"""
    input_files = [ str( f ) for f in input_files ]
    if len( input_files ) < 2:
        raise ValueError( 'List of files to merge must be at least length 2' )
    _impl.merge_files( str( output_file ), '\n'.join( input_files ) )
