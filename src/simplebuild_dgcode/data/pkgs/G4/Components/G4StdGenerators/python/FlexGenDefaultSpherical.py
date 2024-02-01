# A temporary module which we use while migrating

def create():
    #NB: OK-ish with a confluence link here, since only code in
    #dgcode_private_projects is using FlexGenDefaultSpherical.
    print()
    print()
    print(" !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!")
    print(" !! WARNING: Using the FlexGenDefaultSpherical module rather than FlexGen. !!")
    print(" !! You should move back to FlexGen after reading the following page:      !!")
    print(" !! https://confluence.esss.lu.se/x/FgEnC                                  !!")
    print(" !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!")
    print()
    print()

    # For reference, the confluence page above says:
    # It was discovered that the default value of the FlexGen parameter
    # "momdir_spherical" was causing confusion (see DGSW-272 DONE ￼DGSW-314
    # DONE). Therefore the default value was changed from momdir_spherical=True to
    # momdir_spherical=False. However, to be 100% sure as to not cause problems
    # for existing code using FlexGen and somehow (perhaps without realising)
    # relying on the old default value, we created a clone of the FlexGen with the
    # old default value of momdir_spherical and called it
    # FlexGenDefaultSpherical. All existing code in Projects/ using the FlexGen
    # was changed to use FlexGenDefaultSpherical instead.
    #
    # However, we would like to complete the transition by having everyone change
    # their code to using FlexGen instead (perhaps adding a
    # "gen.momdir_spherical=True" in the sim-script at the same time, if
    # needed). Therefore, users of FlexGenDefaultSpherical will see a warning
    # message printed whenever it is used, and are encouraged to change their code
    # to use FlexGen instead (we track this back-migration at ￼DGSW-316 TODO ￼ ).

    import G4StdGenerators.FlexGen as _fg
    gen=_fg.create()
    gen.momdir_spherical = True
    return gen
