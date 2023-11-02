#include "MCNP/sswread.h"
#include <stdio.h>

int main(int argc,char** argv) {
  (void)argc;
  (void)argv;

  if (argc!=2) {
    printf("ERROR: Please supply SSW file to inspect\n");
    return 1;
  }

  ssw_file_t f = ssw_open_file(argv[1]);
  printf("opened ssw file from %s has %lu particles:\n",ssw_mcnpflavour(f),ssw_nparticles(f));

  const ssw_particle_t * p;
  printf("    pdgcode   ekin[MeV]       x[cm]       y[cm]       z[cm]"
         "          ux          uy          uz    time[ns]      weight      isurf\n");
  while ((p=ssw_load_particle(f))) {

    printf("%10li %11.5g %11.5g %11.5g %11.5g"
           " %11.5g %11.5g %11.5g %11.5g %11.5g %10li\n",
           p->pdgcode,p->ekin,p->x,p->y,p->z,
           p->dirx,p->diry,p->dirz,p->time*10,p->weight,p->isurf);
  }

  ssw_close_file(f);

  return 0;
}
