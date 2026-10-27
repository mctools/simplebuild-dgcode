"""Python module for reading .griff files, providing the same interface as the
C++ classes of the GriffDataRead package. It is implemented in pure Python (see
_reader.py).

Example:

  import GriffDataRead
  dr = GriffDataRead.GriffDataReader('myfile.griff')
  while dr.loopEvents():
      for trk in dr.tracks:
          for seg in trk.segments:
              for step in seg.steps:
                  ...
"""

from ._reader import *
from ._reader import __all__
