__all__ = [ 'MplCanvas', 'MplWidget' ]

"""Qt Widget encapsulating matplotlib figure (in canvas and with axes). To use
with Qt designer, promote QWidget to either MplWidget or MplWidgetNoToolbar (can
be done by right-clicking on a QWidget in Qt designer and promoting to
e.g. MplWidget - filling in the name of this python module,
"PyAna.PyQtMPLWidgets", in the so-called "header" field).
"""

from PyQt5 import QtWidgets, QtCore

try:
    import matplotlib
except:
    raise SystemExit('Missing matplotlib module. Perhaps you can install with "python3 -mpip install matplotlib" ?')

try:
    matplotlib.use('QT5Agg')
except ValueError:
    raise SystemExit('Could not select matplotlib backend "QT5Agg")')

import matplotlib.figure
from matplotlib.backends.backend_qt5agg import FigureCanvasQTAgg as FigureCanvas
#from matplotlib.backends.backend_qt5agg import FigureCanvasQT as FigureCanvas
from matplotlib.backends.backend_qt5agg import NavigationToolbar2QT as _NavigationToolbar
#from matplotlib.backends.backend_qt5agg import NavigationToolbar2QTAgg as _NavigationToolbar
from matplotlib.backend_bases import key_press_handler as mpl_key_press_handler

import PyAna.fpe
PyAna.fpe.standardMPLFixes()
PyAna.fpe.disableFPEDuringCall(_NavigationToolbar,'save_figure')

_global_ax_count = [0]
class MplCanvas(FigureCanvas):
    def _add_axis(self):
        global _global_ax_count
        _global_ax_count[0] += 1
        self.ax = self.fig.add_subplot(111,label=f'MplCanvasSubPlot_{_global_ax_count[0]}')
        PyAna.fpe.disableFPEDuringCall(self.ax,'draw')#avoid annoying FPEs in matplotlib drawing code

    def __init__(self):
        self.fig = matplotlib.figure.Figure()
        super().__init__(self.fig)
        self._add_axis()
        self.setSizePolicy(QtWidgets.QSizePolicy.Expanding,
                           QtWidgets.QSizePolicy.Expanding)
        self.updateGeometry()
        #self.setMouseTracking(True)

    def _fix_axis_wo_fig(self):
        #Try to guard against annoying spurious crashes.
        for a in self.fig.axes:
            if not a.figure:
                print(f'WARNING: Axis in figure does not have correct figure set. Attempting to correct (and avoid exception) via hack!')
                a.figure = self.fig
                a.remove()

    def clearAll(self):
        self._fix_axis_wo_fig()
        self.ax=None
        try:
            self.fig.clear()
        except (KeyError,AttributeError) as e:
            print(f'ERROR while clearing plot: {e}')
        #for a in self.fig.axes:
        #    #print("REMOVING axis on",self)
        #    a.remove()
        if self.fig.axes:
            for a in self.fig.axes:
                a.remove()
        if self.fig.axes:
            print('ERROR: Figure still has axes despite trying to remove them!! self.fig.axes=',self.fig.axes)
        assert not self.fig.axes
        self._add_axis()

class MplWidget(QtWidgets.QWidget):
    def __init__(self, parent=None, toolbar=True):
        super().__init__(parent)
        self.canvas = MplCanvas()
        self.vbl = QtWidgets.QVBoxLayout()
        self.vbl.addWidget(self.canvas)
        if toolbar:
            self.toolbar = _NavigationToolbar(self.canvas,self)
            self.vbl.addWidget(self.toolbar)
            self.canvas.setFocusPolicy(QtCore.Qt.StrongFocus)
            self.canvas.mpl_connect('key_press_event', self._on_key_press)
        else:
            self.toolbar = None
        self.setLayout(self.vbl)

    def _on_key_press(self, event):
        assert hasattr(self,'toolbar')
        mpl_key_press_handler(event, self.canvas, self.toolbar)

    def clearAll(self):
        self.canvas._fix_axis_wo_fig()
        self.canvas.clearAll()

    def contentsChanged(self):
        self.canvas._fix_axis_wo_fig()
        self.canvas.draw_idle()

class MplWidgetNoToolbar(MplWidget):
    def __init__(self, parent=None):
        super().__init__(parent=parent,toolbar=False)
