
def bin_centers_and_widths(binedges):
    """turns a list of bin edges into separate lists of bin centers and widths"""
    bin_centers = 0.5*(binedges[1:]+binedges[:-1])
    bin_widths = 0.5*(binedges[1:]-binedges[:-1])
    return bin_centers,bin_widths
