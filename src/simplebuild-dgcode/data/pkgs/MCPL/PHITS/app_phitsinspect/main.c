#include "PHITS/phitsread.h"
#include <stdio.h>

int main(int argc,char** argv) {
  (void)argc;
  (void)argv;

  if (argc!=2) {
    printf("ERROR: Please supply binary PHITS dump file to inspect\n");
    return 1;
  }

  phits_file_t f = phits_open_file(argv[1]);

  printf("opened binary PHITS dump file with contents:\n");

  int haspol = phits_has_polarisation(f);
  const phits_particle_t * p;
  printf("    pdgcode   ekin[MeV]       x[cm]       y[cm]       z[cm]"
         "          ux          uy          uz%s"
         "    time[ns]      weight\n",(haspol?"        polx        poly        polz":""));
  while ((p=phits_load_particle(f))) {
    printf("%10li %11.5g %11.5g %11.5g %11.5g"
           " %11.5g %11.5g %11.5g",
           p->pdgcode,p->ekin,p->x,p->y,p->z,
           p->dirx,p->diry,p->dirz);
    if (haspol)
      printf(" %11.5g %11.5g %11.5g",p->polx,p->poly,p->polz);
    printf(" %11.5g %11.5g\n",p->time,p->weight);
  }

  phits_close_file(f);

  return 0;
}
