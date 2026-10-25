"""Sampling of values from the distribution in a SimpleHists.Hist1D histogram"""

from ._sampler import Sampler as _Sampler

class Sampler:
    """Sampler of values distributed like the contents of a Hist1D histogram
    (optionally scaled by scalefact). Call with a random number in [0,1)."""

    __slots__ = ( '_s', )

    def __init__( self, hist, scalefact = 1.0 ):
        self._s = _Sampler( hist.serialise(), scalefact )

    def reinit( self, hist, scalefact = 1.0 ):
        self._s.reinit( hist.serialise(), scalefact )

    def sample( self, rand ):
        return self._s.sample( rand )

    __call__ = sample

    def sampleMany( self, rand_vals ):
        """Sample values for all random numbers in the numpy array. As before,
        the values are placed in the passed array (which is returned)."""
        rand_vals[:] = self._s.sampleMany( rand_vals )
        return rand_vals
