from __future__ import print_function
# A temporary module which we use while

def create():
    print()
    print()
    print(" !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!")
    print(" !! WARNING: Using the FlexGenDefaultSpherical module rather than FlexGen. !!")
    print(" !! You should move back to FlexGen after reading the following page:      !!")
    print(" !! https://confluence.esss.lu.se/x/FgEnC                                  !!")
    print(" !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!")
    print()
    print()
    import G4StdGenerators.FlexGen as _fg
    gen=_fg.create()
    gen.momdir_spherical = True
    return gen
