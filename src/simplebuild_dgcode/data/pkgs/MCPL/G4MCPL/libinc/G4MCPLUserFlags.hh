#ifndef G4MCPL_G4MCPLUserFlags_hh
#define G4MCPL_G4MCPLUserFlags_hh

#include "G4VUserPrimaryParticleInformation.hh"
#include <cstdint>
class G4Track;
//User information class which will be used to carry userflag info from .mcpl
//files onto Geant4 primary particles (which can be accessed from a primary
//G4Track via the associated G4DynamicParticle object).
//
//For efficiency, only generated particles with userflags!=0 will get an
//information object set.

class G4MCPLUserFlags : public G4VUserPrimaryParticleInformation {
public:

  //Convenience function to access userflags directly via a G4Track pointer
  //(returns 0 both in case of secondary particles or missing userflags):
  static std::uint32_t getFlags(const G4Track*);

public:
  G4MCPLUserFlags(std::uint32_t f) : m_flags(f) {}
  virtual ~G4MCPLUserFlags();
  std::uint32_t userflags() const { return m_flags; }
  //required:
  virtual void Print() const;
private:
  std::uint32_t m_flags;
};



#endif
