__all__ = ['enableNeutronGravity']

import G4GravityHelper.hooks as hooks
import G4StepLimitHelper.helper as slh
import Units

def enableNeutronGravity(launcher, x_dir=0., y_dir=-1., z_dir=0.,g=9.80665,
                         steplim = 10*Units.units.mm):
    ng = hooks.NeutronGravity()
    ng.setDir(x_dir,y_dir,z_dir)
    launcher.preinit_hook(ng.set)

    if steplim:
        slhelper = slh.G4StepLimitHelper()
        slhelper.addWorldLimit(2112,  steplim) #neutron
        launcher.postinit_hook(slhelper.setWorldLimit)

