//Example creating a file with a few different particle species and
//"interesting" energies, which can be used as test input for applications
//needing a semi-plausible test input file.

#include "MCPL/mcpl.h"
#include <stdio.h>
#include <memory.h>
#include <math.h>
#include <assert.h>

int main(int argc,char**argv) {
  if (argc!=2) {
    printf("Please supply output filename\n");
    return 1;
  }
  const char * filename = argv[1];

  mcpl_outfile_t f = mcpl_create_outfile(filename);
  mcpl_hdr_set_srcname(f,"ESS/dgcode/MCPLTests/miscphys");
  mcpl_enable_polarisation(f);
  mcpl_enable_userflags(f);
  mcpl_hdr_add_comment(f,"A simple file with various particle species intended as test input.");

  mcpl_particle_t * particle = mcpl_get_empty_particle(f);

  int pdgcodes[] = {2112,//neutron, [special]
                    2212,//proton,
                    22,//gamma, [special, no anti]
                    11,//electron
                    13,//muon
                    16,//tau neutrino
                    211,//pi+
                    111,//pi0  [no anti]
                    1000010020,//deuteron
                    1000130270,//Al27
                    1000922350,//235-U
                    1000020030,//He-3
                    1000020040};//alpha

  int idx = 0;
  for (unsigned ipdg = 0; ipdg < sizeof(pdgcodes)/sizeof(*pdgcodes); ++ipdg) {
    for (int anti = 0; anti < 2; ++anti) {
      for (int iekin = 0; iekin < 4; ++iekin) {
        for (int iother = 0; iother < 5; ++iother) {
          int pdg = pdgcodes[ipdg];
          if (anti&&(pdg==22||pdg==111))
            continue;//own anti-particle
          if (anti&&(pdg==1000922350||pdg==1000130270))
            continue;//too exotic

          //Reset all fields:
          memset(particle,0,sizeof(*particle));
          particle->weight = 1.0;

          //pick momentum
          double ekin;
          if (iekin==0) {
            if (pdg<-1000||pdg>1000)
              continue;
            ekin = 0.5;// 500 keV/c
          } else if (iekin==1) {
            ekin = 6000.0;// 6 GeV/c
          } else {
            //special ref values for some particles:
            if (pdg==2112) {
              //neutron:
              if (iekin==2) ekin = 81.80420e-9 / (1.8*1.8);//1.8Aa (thermal neutrons)
              else { assert(iekin==3); ekin = 25e-9; }//Ekin=25meV (thermal neutrons)
            } else if (pdg==22) {
              //gamma
              if (iekin==2) ekin = 1.91105769231e-6;//650nm (red light)
              else continue;
            } else if (pdg==13) {
              //muon
              if (iekin==2) ekin = 0;//particle at rest
              else continue;
            } else {
              continue;
            }
          }

          particle->ekin = ekin;

          //pick direction:
          ++idx;
          particle->direction[(idx+2)%3] = (2*(idx%2)-1)*1.0;

          //position:
          particle->position[idx%3] = 10.0;//10cm

          //type:
          particle->pdgcode = (anti?-pdg:pdg);

          //other:
          switch(iother) {
          case 0: break;//nothing
          case 1: particle->polarisation[(idx+2)%3] = 1.0; break;//units???
          case 2: particle->time = 60000.0; break;//1 minute
          case 3: particle->weight = 0.1; break;
          case 4: particle->userflags = 0xdeadbeef; break;
          default:
            assert(0);
          }

          //Finally, add the particle to the file:
          mcpl_add_particle(f,particle);
        }
      }
    }
  }

  //At the end, remember to properly close the output file:
  mcpl_close_outfile(f);
}
