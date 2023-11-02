"""utilities for easy and efficient analysis of .griff data files containing results from Geant4 simulations"""
from __future__ import absolute_import
__author__='thomas.kittelmann@ess.eu'
from . _init import *

class TrackIterator(TrackIterator_cpp):
    def __iter__(self):
        self._reset_cpp()
        while True:
            obj=self._next_cpp()
            if not obj:
                break
            yield obj

class SegmentIterator(SegmentIterator_cpp):
    def __iter__(self):
        self._reset_cpp()
        while True:
            obj=self._next_cpp()
            if not obj:
                break
            yield obj

class StepIterator(StepIterator_cpp):
    def __iter__(self):
        self._reset_cpp()
        while True:
            obj=self._next_cpp()
            if not obj:
                break
            yield obj
