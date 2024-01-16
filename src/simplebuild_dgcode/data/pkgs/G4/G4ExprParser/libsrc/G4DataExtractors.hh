#ifndef G4ExprParser_G4DataExtractors_hh
#define G4ExprParser_G4DataExtractors_hh

#include "ExprParser/Types.hh"
#include "G4MCPL/G4MCPLUserFlags.hh"
#include "G4Step.hh"
#include "G4VProcess.hh"
#include "G4AffineTransform.hh"
#include "G4NavigationHistory.hh"
#include "Utils/NeutronMath.hh"

namespace G4ExprParser {

  namespace DataExtractors {
    using ExprParser::int_type;
    using ExprParser::float_type;
    using ExprParser::str_type;

    //internal helper functions:
    const G4VPhysicalVolume * getvol(const G4StepPoint* sp,int depth)
    {
      auto th = sp->GetTouchableHandle();
      return depth<=th->GetHistoryDepth() ? th->GetVolume(depth) : 0;
    }

    namespace trk {

      //The following functions can be embedded in expressions via the notation
      //"trk.xxx" where xxx is the name of the function below:

      inline int_type trkid(const G4Step* step) { return step->GetTrack()->GetTrackID(); }
      inline int_type parentid(const G4Step* step) { return step->GetTrack()->GetParentID(); }
      inline int_type is_primary(const G4Step* step) { return step->GetTrack()->GetParentID()==0; }
      inline int_type is_secondary(const G4Step* step) { return step->GetTrack()->GetParentID()!=0; }
      inline str_type creatorprocess(const G4Step* step) { auto p = step->GetTrack()->GetCreatorProcess(); return p ? p->GetProcessName() : G4String(); }
      inline float_type weight(const G4Step* step) { return step->GetTrack()->GetWeight(); }
      inline float_type mass(const G4Step* step) { return step->GetTrack()->GetParticleDefinition()->GetPDGMass(); }
      inline float_type width(const G4Step* step) { return step->GetTrack()->GetParticleDefinition()->GetPDGWidth(); }
      inline float_type charge(const G4Step* step) { return step->GetTrack()->GetParticleDefinition()->GetPDGCharge(); }
      inline float_type lifetime(const G4Step* step) { return step->GetTrack()->GetParticleDefinition()->GetPDGLifeTime(); }
      inline str_type name(const G4Step* step) { return step->GetTrack()->GetParticleDefinition()->GetParticleName(); }
      inline str_type type(const G4Step* step) { return step->GetTrack()->GetParticleDefinition()->GetParticleType(); }
      inline str_type subtype(const G4Step* step) { return step->GetTrack()->GetParticleDefinition()->GetParticleSubType(); }
      inline int_type atomicnumber(const G4Step* step) { return step->GetTrack()->GetParticleDefinition()->GetAtomicNumber(); }
      inline int_type atomicmass(const G4Step* step) { return step->GetTrack()->GetParticleDefinition()->GetAtomicMass(); }
      inline float_type magneticmoment(const G4Step* step) { return step->GetTrack()->GetParticleDefinition()->GetPDGMagneticMoment(); }
      inline float_type spin(const G4Step* step) { return step->GetTrack()->GetParticleDefinition()->GetPDGSpin(); }
      inline int_type is_stable(const G4Step* step) { return step->GetTrack()->GetParticleDefinition()->GetPDGStable(); }
      inline int_type is_shortlived(const G4Step* step) { return step->GetTrack()->GetParticleDefinition()->IsShortLived(); }
      inline int_type pdgcode(const G4Step* step) { return step->GetTrack()->GetDynamicParticle()->GetPDGcode(); }//nb: optical photons have 0
      inline int_type is_neutron(const G4Step* step) { return pdgcode(step)==2112; }
      inline int_type is_neutrino(const G4Step* step)
      {
        auto pp = std::abs(pdgcode(step));
        return pp==12||pp==14||pp==16;
      }
      inline int_type is_opticalphoton(const G4Step* step)
      {
        auto pdg=step->GetTrack()->GetDynamicParticle()->GetPDGcode();
        return (pdg==0||pdg==22) && step->GetTrack()->GetParticleDefinition()->GetParticleName() == "opticalphoton";
      }
      inline int_type is_photon(const G4Step* step)
      {
        //standard photon or optical photon
        auto pdg=step->GetTrack()->GetDynamicParticle()->GetPDGcode();
        return pdg==22 || (pdg==0 && step->GetTrack()->GetParticleDefinition()->GetParticleName() == "opticalphoton");
      }
      inline int_type is_gamma(const G4Step* step)
      {
        return is_photon(step);
      }
      inline int_type is_ion(const G4Step* step)
      {
        auto pp = std::abs(pdgcode(step));
        return pp/100000000 == 10;
      }

      inline int_type mcplflag(const G4Step* step) { return G4MCPLUserFlags::getFlags(step->GetTrack()); }

    }

    namespace step {

      //The following functions can be embedded in expressions via the notation
      //"step.xxx" where xxx is the name of the function below:

      inline float_type edep(const G4Step* step) { return step->GetTotalEnergyDeposit(); }
      inline float_type edep_nonion(const G4Step* step) { return step->GetNonIonizingEnergyDeposit(); }
      inline float_type edep_ion(const G4Step* step) { return step->GetTotalEnergyDeposit()-step->GetNonIonizingEnergyDeposit(); }
      inline float_type delta_e(const G4Step* step) { return step->GetDeltaEnergy(); }
      inline float_type delta_mom(const G4Step* step) { return step->GetDeltaMomentum().mag(); }
      inline float_type delta_pos(const G4Step* step) { return step->GetDeltaPosition().mag(); }
      inline float_type delta_time(const G4Step* step) { return step->GetDeltaTime(); }
      inline float_type steplength(const G4Step* step) { return step->GetStepLength(); }
      inline int_type stepnbr(const G4Step* step) { return step->GetTrack()->GetCurrentStepNumber(); }
      inline str_type volname(const G4Step* step) { auto v = getvol(step->GetPreStepPoint(),0); return v ? v->GetName() : G4String(); }
      inline str_type volname_1(const G4Step* step) { auto v = getvol(step->GetPreStepPoint(),1); return v ? v->GetName() : G4String(); }
      inline str_type volname_2(const G4Step* step) { auto v = getvol(step->GetPreStepPoint(),2); return v ? v->GetName() : G4String(); }
      inline int_type volcopyno(const G4Step* step) { auto v = getvol(step->GetPreStepPoint(),0); return v ? v->GetCopyNo() : 0; }
      inline int_type volcopyno_1(const G4Step* step) { auto v = getvol(step->GetPreStepPoint(),0); return v ? v->GetCopyNo() : 0; }
      inline int_type volcopyno_2(const G4Step* step) { auto v = getvol(step->GetPreStepPoint(),0); return v ? v->GetCopyNo() : 0; }
      inline str_type mat_name(const G4Step* step) { auto m = step->GetPreStepPoint()->GetMaterial(); return m ? m->GetName() : G4String(); }
      inline int_type mat_state(const G4Step* step) { auto m = step->GetPreStepPoint()->GetMaterial(); return m ? m->GetState() : kStateUndefined; }//NB: can compare with constants like "kStateSolid" in expressions
      inline float_type mat_density(const G4Step* step) { auto m = step->GetPreStepPoint()->GetMaterial(); return m ? m->GetDensity() : 0.0; }
      inline float_type mat_temperature(const G4Step* step) { auto m = step->GetPreStepPoint()->GetMaterial(); return m ? m->GetTemperature() : 0.0; }
      inline float_type mat_pressure(const G4Step* step) { auto m = step->GetPreStepPoint()->GetMaterial(); return m ? m->GetPressure() : 0.0; }

      inline str_type process_defined_step(const G4Step* step) { auto p = step->GetPostStepPoint()->GetProcessDefinedStep(); return p ? p->GetProcessName() : G4String(); }

      //This special function is accessed via variables "step.pre.neutron_wl or step.post.neutron_wl":
      template<bool isprestep>
      inline float_type neutron_wl(const G4Step* step) {
        //TODO: We should simply have a wavelength variable working for all particles, not just neutrons!
        if (!trk::is_neutron(step)) {
          EXPRPARSER_THROW2(DomainError,"step."<<(isprestep?"pre":"post")<<".neutron_wl must only be called for neutrons"
                            " (use \"trk.is_neutron\" in your expression to make sure,"
                            " like \"trk.is_neutron && step."<<(isprestep?"pre":"post")<<".neutron_wl > 2Aa\")");
        }
        const G4StepPoint* sp = isprestep ? step->GetPreStepPoint() : step->GetPostStepPoint();
        return Utils::neutronEKinToWavelength(sp->GetKineticEnergy());
      }
    }

    namespace steppoint {

      //The following functions can be reached in expressions via either of the
      //notations "step.pre.xxx" or "step.post.xxx" where xxx is the name of the
      //function below, and where "pre" or "post" is used depending on whether
      //the function is to be evaluated on the PreStepPoint() or the
      //PostStepPoint() of the G4Step:

      inline int_type stepstatus(const G4StepPoint* sp) { return sp->GetStepStatus(); }//NB: can compare with constants like "fGeomBoundary" in expressions
      inline int_type exists(const G4StepPoint* sp) { return sp!=nullptr; }
      inline float_type time(const G4StepPoint* sp) { return sp->GetGlobalTime(); }
      inline float_type globaltime(const G4StepPoint* sp) { return sp->GetGlobalTime(); }
      inline float_type localtime(const G4StepPoint* sp) { return sp->GetLocalTime(); }
      inline float_type propertime(const G4StepPoint* sp) { return sp->GetProperTime(); }
      inline str_type mat_name(const G4StepPoint* sp) { auto m = sp->GetMaterial(); return m ? m->GetName() : G4String(); }
      inline int_type mat_state(const G4StepPoint* sp) { auto m = sp->GetMaterial(); return m ? m->GetState() : kStateUndefined; }//NB: can compare with constants like "kStateSolid" in expressions
      inline float_type mat_density(const G4StepPoint* sp) { auto m = sp->GetMaterial(); return m ? m->GetDensity() : 0.0; }
      inline float_type mat_temperature(const G4StepPoint* sp) { auto m = sp->GetMaterial(); return m ? m->GetTemperature() : 0.0; }
      inline float_type mat_pressure(const G4StepPoint* sp) { auto m = sp->GetMaterial(); return m ? m->GetPressure() : 0.0; }
      inline float_type ekin(const G4StepPoint* sp) { return sp->GetKineticEnergy(); }
      inline float_type etot(const G4StepPoint* sp) { return sp->GetTotalEnergy(); }
      inline float_type velocity(const G4StepPoint* sp) { return sp->GetVelocity(); }
      inline float_type beta(const G4StepPoint* sp) { return sp->GetBeta(); }
      inline float_type gamma(const G4StepPoint* sp) { return sp->GetGamma(); }
      inline int_type at_voledge(const G4StepPoint* sp) { auto st = sp->GetStepStatus(); return ( st==fGeomBoundary || st==fWorldBoundary ); }
      inline float_type x(const G4StepPoint* sp) { return sp->GetPosition().x(); }
      inline float_type y(const G4StepPoint* sp) { return sp->GetPosition().y(); }
      inline float_type z(const G4StepPoint* sp) { return sp->GetPosition().z(); }
      inline float_type px(const G4StepPoint* sp) { return sp->GetMomentum().x(); }
      inline float_type py(const G4StepPoint* sp) { return sp->GetMomentum().y(); }
      inline float_type pz(const G4StepPoint* sp) { return sp->GetMomentum().z(); }
      inline float_type polx(const G4StepPoint* sp) { return sp->GetPolarization().x(); }
      inline float_type poly(const G4StepPoint* sp) { return sp->GetPolarization().y(); }
      inline float_type polz(const G4StepPoint* sp) { return sp->GetPolarization().z(); }
      inline float_type mom(const G4StepPoint* sp) { return sp->GetMomentum().mag(); }
      inline float_type mom2(const G4StepPoint* sp) { return sp->GetMomentum().mag2(); }
      inline float_type weight(const G4StepPoint* sp) { return sp->GetWeight(); }
      inline str_type process_defined_step(const G4StepPoint* sp) { auto p = sp->GetProcessDefinedStep(); return p ? p->GetProcessName() : G4String(); }
      inline str_type volname(const G4StepPoint* sp) { auto v = getvol(sp,0); return v ? v->GetName() : G4String(); }
      inline str_type volname_1(const G4StepPoint* sp) { auto v = getvol(sp,1); return v ? v->GetName() : G4String(); }
      inline str_type volname_2(const G4StepPoint* sp) { auto v = getvol(sp,2); return v ? v->GetName() : G4String(); }
      inline int_type volcopyno(const G4StepPoint* sp) { auto v = getvol(sp,0); return v ? v->GetCopyNo() : 0; }
      inline int_type volcopyno_1(const G4StepPoint* sp) { auto v = getvol(sp,0); return v ? v->GetCopyNo() : 0; }
      inline int_type volcopyno_2(const G4StepPoint* sp) { auto v = getvol(sp,0); return v ? v->GetCopyNo() : 0; }
      //TODO: Safer to always use the transform from the presteppoint:
      inline float_type local_x(const G4StepPoint* sp)
      {
        return (sp->GetTouchableHandle()->GetHistory()->GetTopTransform().TransformPoint(sp->GetPosition())).x();
      }
      inline float_type local_y(const G4StepPoint* sp)
      {
        return (sp->GetTouchableHandle()->GetHistory()->GetTopTransform().TransformPoint(sp->GetPosition())).y();
      }
      inline float_type local_z(const G4StepPoint* sp)
      {
        return (sp->GetTouchableHandle()->GetHistory()->GetTopTransform().TransformPoint(sp->GetPosition())).z();
      }

    }
  }
}

#endif
