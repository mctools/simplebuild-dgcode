"""If installed, will install a post-generation hook that will duplicate all
particles as geantinoes. Can be used to test where the primary particles would
have intersected the geometry, in case there were no interactions."""

def install():
    """Install the hook on the launcher (which must already be created)"""
    import G4Launcher
    from G4Interfaces.ToPy import CppObject
    from ._impl import create_hook
    launcher = G4Launcher.getTheLauncher()
    if launcher is None:
        raise RuntimeError('G4GeantinoInserter.install called before G4Launcher object was created')
    launcher.addPostGenHook( CppObject( create_hook() ) )
