__doc__='python module for package G4DataCollect'
__all__ = [ 'finish', 'installHooks', 'setMetaData', 'setUserData' ]

from ._impl import finish, setMetaData, setUserData

def installHooks( outputFile, mode = 'FULL' ):
    """Installs hooks necessary to record the output of the Geant4 simulation."""
    from ._impl import installHooks as _impl
    _impl( outputFile, mode )
