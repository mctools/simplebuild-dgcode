__doc__ = 'Writing of heatmaps (mesh3d files) with quantities extracted from simulation steps'

__all__ = [ 'HeatMapWriter' ]

_full_args = ( 'filename', 'nx', 'xlow', 'xup', 'ny', 'ylow', 'yup', 'nz', 'zlow', 'zup' )
_auto_args = ( 'filename', 'nx', 'ny', 'nz' )

def _bind( names, args, kwargs ):
    if len( args ) > len( names ):
        return None
    d = dict( zip( names, args ) )
    for k, v in kwargs.items():
        if k not in names or k in d:
            return None
        d[k] = v
    return [ d[n] for n in names ] if len( d ) == len( names ) else None

class HeatMapWriter:
    """Writer of a heatmap to a mesh3d file. Construct with either (filename,
    nx, xlow, xup, ny, ylow, yup, nz, zlow, zup) or (filename, nx, ny, nz), in
    which case the limits are based on the extent of the geometry. The inithook
    method must be added as a postinit hook on the launcher."""

    def __init__( self, *args, **kwargs ):
        from . import _impl
        a = _bind( _full_args, args, kwargs )
        if a is not None:
            self._w = _impl.HeatMapWriter( *a )
            return
        a = _bind( _auto_args, args, kwargs )
        if a is None:
            raise TypeError( 'HeatMapWriter requires the arguments (%s) or (%s)'
                             %( ', '.join( _full_args ), ', '.join( _auto_args ) ) )
        self._w = _impl.create_auto( *a )

    def inithook( self ):
        """Hook to add as a postinit hook on the launcher"""
        import G4Launcher
        from G4Interfaces.ToPy import CppObject
        launcher = G4Launcher.getTheLauncher()
        stepact, evtact = self._w.inithook()
        if stepact:
            launcher.setUserSteppingAction( CppObject( stepact ) )
            launcher.setUserEventAction( CppObject( evtact ) )
        #Merge callback (will only ever get invoked in parent proc in true MP jobs):
        launcher.postmp_hook( self._w.merge )

    def setQuantity( self, expression ):
        self._w.setQuantity( expression )

    def setFilter( self, expression ):
        self._w.setFilter( expression )

    def setComments( self, comments ):
        self._w.setComments( comments )
