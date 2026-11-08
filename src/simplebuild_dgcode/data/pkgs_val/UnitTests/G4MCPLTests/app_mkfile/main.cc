//Create MCPL files for the tests of G4MCPLPlugins (the stat:sum handling in
//MCPLGen and MCPLWriter). Usage:
//
//   sb_g4mcpltests_mkfile FILENAME NPARTICLES WEIGHT INITIAL_RAY_COUNT
//
//A value of "none" for INITIAL_RAY_COUNT means that the file does not get a
//stat:sum:initial_ray_count, while "notfinal" leaves the value at -1 (i.e. not
//available).

#include "mcpl.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>

int main( int argc, char** argv )
{
  if ( argc != 5 ) {
    printf("Usage: %s FILENAME NPARTICLES WEIGHT INITIAL_RAY_COUNT\n",argv[0]);
    return 1;
  }
  const char * irc = argv[4];
  mcpl_outfile_t f = mcpl_create_outfile( argv[1] );
  mcpl_hdr_set_srcname( f, "sb_g4mcpltests_mkfile" );
  if ( std::strcmp( irc, "none" ) != 0 )
    mcpl_hdr_add_stat_sum( f, "initial_ray_count",
                           std::strcmp( irc, "notfinal" ) == 0 ? -1.0 : std::atof( irc ) );
  mcpl_particle_t * p = mcpl_get_empty_particle( f );
  const int n = std::atoi( argv[2] );
  for ( int i = 0; i < n; ++i ) {
    p->pdgcode = 2112;
    p->ekin = 0.025e-6 * ( 1 + i );//MeV
    p->direction[0] = p->direction[1] = 0.0;
    p->direction[2] = 1.0;
    p->position[0] = 0.1 * i;
    p->weight = std::atof( argv[3] );
    mcpl_add_particle( f, p );
  }
  mcpl_close_outfile( f );
  return 0;
}
