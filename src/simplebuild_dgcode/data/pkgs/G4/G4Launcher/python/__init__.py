#We use a little class/sys.modules trick to allow both: G4Launcher(..) and
#G4Launcher.Launcher(..)

class Launcher:
    def __init__(self):
        """<internal>"""
        import G4Launcher._launcher
        self.__Launcher = G4Launcher._launcher.Launcher
        self.__getTheLauncher = G4Launcher._launcher.getTheLauncher
        self.__g4version = G4Launcher._launcher.g4version()
    @property
    def __name__( self ):
        return 'Launcher'
    def Launcher(self,*args):
        """Create and return Launcher object. Arguments can contain a geometry,
        generator, and/or a Griff step filter. Alternatively set them later with
        the .setGeo/.setGen/.setFilter methods."""
        l = self.__Launcher( list(e for e in args) )
        for e in args:
            l._addResourceGuard(e)
        return l
    def __call__(self,*args):
        """Create and return Launcher object. Arguments can contain a geometry,
        generator, and/or a Griff step filter. Alternatively set them later with
        the .setGeo/.setGen/.setFilter methods."""
        return self.Launcher(*args)
    def getTheLauncher(self):
        """Access already instantiated Launcher object."""
        return self.__getTheLauncher()
    def g4version(self):
        """Returns g4 version as integer (for instance 1103 means version 11.0.p03)"""
        return self.__g4version

import sys
sys.modules[__name__] = Launcher()
