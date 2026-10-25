#For backwards compatibility with code doing "from G4Launcher._init import
#g4version" (G4Launcher._init used to be a compiled module):
from ._impl import g4version, g4versionstr
