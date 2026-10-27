__all__ = [ 'dumpFileEventBriefDataSection', 'dumpFileEventDBSection',
            'dumpFileEventFullDataSection', 'dumpFileInfo' ]

from ._dumpfile import ( dumpFileEventDBSection,
                         dumpFileEventBriefDataSection,
                         dumpFileEventFullDataSection )

def dumpFileInfo( filename, brief = False, show_uncompressed_sizes = False ):
    """dump content of G4 event file"""
    from ._dumpfile import dumpFileInfo as _impl
    return _impl( filename, brief, show_uncompressed_sizes )
