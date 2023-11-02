def floateq(a,b,rtol=1.0e-6,atol=1.0e-6):
    return abs(a-b) <= 0.5 * rtol * (abs(a) + abs(b)) + atol
