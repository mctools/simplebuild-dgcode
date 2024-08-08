#To avoid confusing users with useless(?) warnings such as the following:
#
#/usr/lib64/python2.7/site-packages/matplotlib/backends/backend_gtk.py:250: Warning: Source ID 2 was not found when attempting to remove it
#  gobject.source_remove(self._idle_event_id)
#
#we monkey-patch FigureCanvasGTK.destroy and block stderr while it is invoked.
#
#Hopefully in some future version of matplotlib this warning will disappear by itself.
import sys
import os
try:
    from matplotlib.backends.backend_gtk import FigureCanvasGTK
    if hasattr(FigureCanvasGTK,'destroy'):
        _FigureCanvasGTK_destroy = FigureCanvasGTK.destroy
        def patched_FigureCanvasGTK_destroy(self):
            _stderr = sys.stderr
            null = open(os.devnull,'wb')
            try:
                sys.stderr.flush()
                sys.stderr = null
                _FigureCanvasGTK_destroy(self)
            finally:
                sys.stderr.flush()
                sys.stderr = _stderr
                null.close()
        FigureCanvasGTK.destroy=patched_FigureCanvasGTK_destroy
except ImportError:
    #Something unexpected, but let us not break stuff for users because of it!
    pass
