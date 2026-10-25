__all__ = [ 'MCPLWriter' ]

class MCPLWriter:
    """Capture particles in a Geant4 simulation and write them to an MCPL file.
    After configuration, the inithook method must be added as a postinit hook on
    the launcher."""

    def __init__( self, filename ):
        from ._MCPLWriter import MCPLWriter as _MCPLWriter
        self._w = _MCPLWriter( filename )
        for n in ( 'addVolume', 'setFilter', 'setUserFlags', 'setWriteOnVolExit',
                   'setWriteDoublePrecision', 'setWritePolarisation', 'setWriteUserFlags',
                   'setUniversalWeight', 'setKillStrategy' ):
            setattr( self, n, getattr( self._w, n ) )

    def inithook( self ):
        """Hook to add as a postinit hook on the launcher"""
        import G4Launcher
        from G4Launcher._launcher import _component_handle
        launcher = G4Launcher.getTheLauncher()
        def handle( c ):
            return ( _component_handle( c ) or 0 ) if c is not None else 0
        self._w.inithook( handle( launcher.getGeo() ), handle( launcher.getGen() ) )
        #Merging of output files (postmp hooks are only invoked in the parent
        #process of multi-process jobs):
        launcher.postmp_hook( self._w.postmp )
