def hash2seed(astr):
    import hashlib
    seed = int(hashlib.md5(astr.encode() if hasattr(astr,'encode') else astr).hexdigest(), 16)
    return seed%4294967295

def hash2seedpar(astr):
    return '--seed=%i'%hash2seed(astr)

