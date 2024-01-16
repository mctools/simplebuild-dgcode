#include "G4MCPL/G4MCPLUserFlags.hh"
#include <cassert>
#include "G4Track.hh"
#include "G4DynamicParticle.hh"
#include "G4PrimaryParticle.hh"
#include "G4ios.hh"
#include <cstdio>

std::uint32_t G4MCPLUserFlags::getFlags(const G4Track* trk)
{
  assert(trk);
  const G4DynamicParticle* dp = trk->GetDynamicParticle();
  G4PrimaryParticle* pp = dp ? dp->GetPrimaryParticle() : 0;
  if (!pp)//not a primary track
    return 0;
  G4MCPLUserFlags* ui = dynamic_cast<G4MCPLUserFlags*>(pp->GetUserInformation());
  return ui ? ui->userflags() : 0;
}

G4MCPLUserFlags::~G4MCPLUserFlags()
{
}

void G4MCPLUserFlags::Print() const
{
  char hexval[9];
#ifndef NDEBUG
  int np =
#endif
    snprintf ( hexval, 9,  "%08x", m_flags);
  assert(np==8);
  G4cout << "G4MCPLUserFlags[0x"<<hexval<<"]";
}
