__all__ = [ 'run', 'run_nogeom' ]

def run( griff_file = None, events = None ):
    """Launch the viewer, with the events in the Griff file if one is given (all
    events unless a list of event numbers is given)."""
    from . import _Viewer
    if griff_file is None:
        if events is not None:
            raise ValueError( 'events can only be specified along with griff_file' )
        return _Viewer.run()
    return _Viewer.run_griff( griff_file, list( events or [] ) )

def run_nogeom( griff_file, events = None ):
    """Launch the viewer with the events in the Griff file but without the
    geometry (all events unless a list of event numbers is given)."""
    from . import _Viewer
    return _Viewer.run_griff_nogeom( griff_file, list( events or [] ) )
