import ROOT

def list2TVectorD(l):
  v=ROOT.TVectorD(len(l))
  for i,x in enumerate(l):
    v[i]=x
  return v
