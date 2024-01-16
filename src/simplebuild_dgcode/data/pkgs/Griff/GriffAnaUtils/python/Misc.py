from __future__ import division

def vector_costheta(mom1,mom2):
    import math
    assert len(mom1)==3 and len(mom2)==3
    sq1=sum(e*e for e in mom1)
    sq2=sum(e*e for e in mom2)
    assert sq1>0 and sq2>0
    return float(mom1[0]*mom2[0]+mom1[1]*mom2[1]+mom1[2]*mom2[2])/math.sqrt(sq1*sq2)

def momdir_costhetha(trk1,trk2):
    s1=trk1.firstStep()
    s2=trk2.firstStep()
    return vector_costheta((s1.preMomentumX(),s1.preMomentumY(),s1.preMomentumZ()),
                           (s2.preMomentumX(),s2.preMomentumY(),s2.preMomentumZ()))

def momdir_vector(trk):
    #needs step...
    s=trk.firstStep()
    return (s.preMomentumX(),s.preMomentumY(),s.preMomentumZ())

def prettify_particlenames(s):
    return s.replace('gamma','#gamma').replace('alpha','#alpha').replace('Li7[0.0]','Li^{7}').replace('B11[0.0]','B^{11}')

def get_daughters_recursively(trk):
    for d in trk.daughters:
        yield d
        for d2 in get_daughters_recursively(d):
            yield d2
