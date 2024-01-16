"""python module for package GriffDataRead"""
from __future__ import absolute_import
from . _init import *

def _Track_daughter_iter(self):
    i=0
    n=self.nDaughters()
    while i<n:
        yield(self.getDaughter(i))
        i+=1
Track.daughters = property(_Track_daughter_iter)

#NB: The use of "property" so we can for instance write "trk.daughters" instead of "trk.daughters()":
#    def getx(self):
#        return self._x
#    def setx(self, value):
#        self._x = value
#    def delx(self):
#        del self._x
#    x = property(getx, setx, delx, "I'm the 'x' property.")

def _Track_segment_iter(self):
    i=0
    n=self.nSegments()
    while i<n:
        yield(self.getSegment(i))
        i+=1
Track.segments = property(_Track_segment_iter)

def _Segment_step_iter(self):
    i=0
    n=self.nStepsStored()
    while i<n:
        yield(self.getStep(i))
        i+=1
Segment.steps = property(_Segment_step_iter)

def _dr_primtrk_iter(self):
    i=0
    n=self.nTracks()
    while i<n:
        t=self.getTrack(i)
        if not t.isPrimary():
            break#assume that all primary tracks are at the beginning
        yield t
        i+=1
GriffDataReader.primaryTracks = property(_dr_primtrk_iter)

def _dr_trk_iter(self):
    i=0
    n=self.nTracks()
    while i<n:
        t=self.getTrack(i)
        yield t
        i+=1
GriffDataReader.tracks = property(_dr_trk_iter)
