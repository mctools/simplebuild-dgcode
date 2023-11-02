#include "MCPL/mcpl.h"
#include <stdio.h>
#include <memory.h>

int main(int argc,char**argv) {
  (void)argc;
  (void)argv;

  //Create file with some rather fake content:
  mcpl_outfile_t f = mcpl_create_outfile("out.mcpl");
  mcpl_hdr_set_srcname(f,"CustomTest");
  mcpl_enable_universal_pdgcode(f,2112);//all particles are neutrons
  mcpl_hdr_add_comment(f,"Some comment.");
  mcpl_hdr_add_comment(f,"Another comment.");
  mcpl_particle_t * particle = mcpl_get_empty_particle(f);
  for (int i = 0; i < 50000; ++i) {
    particle->position[0] = i*1.0;
    particle->position[1] = i*2.0;
    particle->position[2] = i*3.0;
    particle->ekin = i*0.1;
    particle->direction[0] = 0.0;
    particle->direction[1] = 0.0;
    particle->direction[2] = 1.0;
    particle->time = i*0.01;
    particle->weight = 1.0;
    mcpl_add_particle(f,particle);
  }

  //close up and gzip:
  mcpl_closeandgzip_outfile(f);

  //Check file:
  mcpl_dump("out.mcpl.gz", 0,0,20);

}
