import G4CustomPyGen
from Units import units as Units
import math

class CorrelatedBeamGen(G4CustomPyGen.GenBase):
    """Generator which produces a beam of neutrons whose profile
    has a given Gaussian x and y spread, and with a given level
    of correlation"""

    def declare_parameters(self):
        self.addParameterDouble("spread_x_mm",5.0)
        self.addParameterDouble("spread_y_mm",10.0)
        self.addParameterDouble("correlation",0.8,0.0,1.0)
        self.addParameterDouble("neutron_wavelength_aa",1.8)

    def init_generator(self,gun):
        gun.set_type('neutron')
        gun.set_direction(0,0,1)
        gun.set_wavelength_angstrom(self.neutron_wavelength_aa)

    def generate_event(self,gun):
        x1 = self.randGauss()
        x2 = self.randGauss()
        c = self.correlation
        x3 = c*x1 - math.sqrt(1-c*c)*x2
        x = x1*self.spread_x_mm*Units.mm
        y = x3*self.spread_y_mm*Units.mm
        gun.set_position( x, y, 0)



class GammaExpGen(G4CustomPyGen.GenBase):
    """Generator which produces a beam of gammas whose energy is given by an
    exponential distribution with a user supplied average"""

    def declare_parameters(self):
        self.addParameterDouble("average_energy_keV",50.0)

    def init_generator(self,gun):
        gun.set_type('gamma')
        gun.set_direction(0,0,1)
        gun.set_position(0,0,0)

    def generate_event(self,gun):
        gun.set_energy(self.randExponential() * self.average_energy_keV*Units.keV)



class MultiGammaGen(G4CustomPyGen.GenBase):
    """Generator which produces random number of gammas of a given energy"""

    def declare_parameters(self):
        self.addParameterDouble("average_ngamma",2.5)

    def init_generator(self,gun):
        gun.set_type('gamma')
        gun.set_direction(0,0,1)
        gun.set_position(0,0,0)
        gun.set_energy(1*Units.MeV)
        #to prevent automatic calls to gun.fire() when the poisson dist below
        #selects 0 particles, we must explicitly allow empty events like this:
        gun.allow_empty_events()

    def generate_event(self,gun):
        for i in range(self.randPoisson(self.average_ngamma)):
            gun.fire()

class AlphaHistGen(G4CustomPyGen.GenBase):
    """Generator which produces an alpha particle with an energy sampled from a histogram"""

    def declare_parameters(self):
        self.addParameterString("energy_histogram",'G4CustomPyGen/example.shist:alpha_energy:keV')

    def init_generator(self,gun):
        gun.set_type('alpha')
        gun.set_direction(0,0,1)
        gun.set_position(0,0,0)
        self._esampler = self.create_hist_sampler(self.energy_histogram)

    def generate_event(self,gun):
        gun.set_energy(self._esampler())


class LimitedGen(G4CustomPyGen.GenBase):
    """Example of generator which produces a finite amount of events. Here we want N
    points equidistant in some interval, but another use-case might be that of
    an external particle source with limited statistics."""

    def declare_parameters(self):
        self.addParameterDouble("e_min_MeV",0.0)
        self.addParameterDouble("e_max_MeV",1.0)
        self.addParameterInt("n_points",100)

    def init_generator(self,gun):
        gun.set_type('geantino')
        gun.set_direction(0,0,1)
        gun.set_position(0,0,0)
        self._i = 0
        self._emin = self.e_min_MeV * Units.MeV
        self._de = (self.e_max_MeV-self.e_min_MeV)*Units.MeV/self.n_points

    def unlimited(self):
        return False

    def generate_event(self,gun):
        self._i += 1
        gun.set_energy(self._emin + self._i*self._de)
        if self._i == self.n_points:
            self.signalEndOfEvents(False)#False meaning that current event is ok


