# python module for printing numpy arrays of floating point numbers in a
# reproducible way for unit tests

_fmt = '%.8g'
def _fmtitems(items):
    return '  '.join(_fmt%f for f in items)

def format_numpy_1darray_asfloat(a,edgeitems=3,threshold=1000):
    if not hasattr(a,'shape') or len(a.shape)!=1:
        return str(a)
    if len(a)>threshold:
        return '[ %s ... %s ]'%(_fmtitems(a[0:edgeitems]),_fmtitems(a[-edgeitems:]))
    else:
        return '[ %s ]'%(_fmtitems(a))


