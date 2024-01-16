#Note that numpy provide more efficient functionality than this!
def parse(filename):
  data={}
  columnindex2columnname={}
  for l in open(filename):
    if not data:
      for i,columnname in enumerate(l.split()):
        columnindex2columnname[i] = columnname
        data[columnname] = []
      continue
    for i,value in enumerate(l.split()):
      data[columnindex2columnname[i]] += [float(value)]
  return data
