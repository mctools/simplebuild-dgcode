#include "G4ExprParser/G4SteppingASTBuilder.hh"
#include "G4DataExtractors.hh"
#include "ExprParser/ASTStdPhys.hh"
#include "Core/String.hh"
#include "G4Step.hh"
#include "G4Material.hh"//G4State enum

namespace G4ExprParser {

  template<class TValue>
  class G4EEValBase : public ExprParser::ExprEntity<TValue> {
  public:
    G4EEValBase(const str_type name_, const G4Step *& step)
      : ExprParser::ExprEntity<TValue>(), m_step(step), m_name(name_) {}
    virtual bool isConstant() const { return false; }
    virtual str_type name() const { return m_name; }
  protected:
    const G4Step *& m_step;
    str_type m_name;
  };

  template<class TValue, TValue eval_func(const G4Step*)>
  class G4EEVal final : public G4EEValBase<TValue> {
  public:
    typedef G4EEValBase<TValue> TSuper;
    using TSuper::TSuper;
    virtual TValue evaluate() const
    {
      assert(TSuper::m_step&&"did you remember to call G4SteppingASTBuilder::setCurrentStep before evaluating the expression?");
      return eval_func(TSuper::m_step);
    }
  };

  template <int_type thefunc(const G4Step*)>
  ExprEntityPtr wrap_extractor(const str_type& name, const G4Step *& stepref)
  {
    return ExprParser::makeobj<G4EEVal<decltype(thefunc(0)),thefunc>>(name,stepref);
  }
  template <float_type thefunc(const G4Step*)>
  ExprEntityPtr wrap_extractor(const str_type& name, const G4Step *& stepref)
  {
    return ExprParser::makeobj<G4EEVal<decltype(thefunc(0)),thefunc>>(name,stepref);
  }
  template <str_type thefunc(const G4Step*)>
  ExprEntityPtr wrap_extractor(const str_type& name, const G4Step *& stepref)
  {
    return ExprParser::makeobj<G4EEVal<decltype(thefunc(0)),thefunc>>(name,stepref);
  }

  template <class type_res, type_res fct(const G4StepPoint*)> type_res wrap_prestep_extractor(const G4Step* step)
  { assert(step->GetPreStepPoint()); return fct(step->GetPreStepPoint()); }
  template <class type_res, type_res fct(const G4StepPoint*)> type_res wrap_poststep_extractor(const G4Step* step)
  { assert(step->GetPostStepPoint()); return fct(step->GetPostStepPoint()); }

  template <int_type thefunc(const G4StepPoint*)>
  ExprEntityPtr wrap_extractor(const str_type& name, const G4Step *& stepref, bool is_poststep)
  {
    typedef decltype(thefunc(0)) type_res;
    return is_poststep ? ExprParser::makeobj<G4EEVal<type_res,wrap_poststep_extractor<type_res,thefunc>>>(name,stepref)
                       : ExprParser::makeobj<G4EEVal<type_res,wrap_prestep_extractor<type_res,thefunc>>>(name,stepref);
  }
  template <float_type thefunc(const G4StepPoint*)>
  ExprEntityPtr wrap_extractor(const str_type& name, const G4Step *& stepref, bool is_poststep)
  {
    typedef decltype(thefunc(0)) type_res;
    return is_poststep ? ExprParser::makeobj<G4EEVal<type_res,wrap_poststep_extractor<type_res,thefunc>>>(name,stepref)
                       : ExprParser::makeobj<G4EEVal<type_res,wrap_prestep_extractor<type_res,thefunc>>>(name,stepref);
  }
  template <str_type thefunc(const G4StepPoint*)>
  ExprEntityPtr wrap_extractor(const str_type& name, const G4Step *& stepref, bool is_poststep)
  {
    typedef decltype(thefunc(0)) type_res;
    return is_poststep ? ExprParser::makeobj<G4EEVal<type_res,wrap_poststep_extractor<type_res,thefunc>>>(name,stepref)
                       : ExprParser::makeobj<G4EEVal<type_res,wrap_prestep_extractor<type_res,thefunc>>>(name,stepref);
  }


  ExprEntityPtr G4SteppingASTBuilder::createValue(const str_type& name) const
  {
    using ExprParser::makeobj;
    using ExprParser::create_constant;

    //standard math/physics constants:

    auto p = ASTBuilder::createValue(name);
    p = p ? p : ExprParser::create_standard_unit_or_constant(name);
    if (p)
      return p;

    //Special G4 constants:

    if (!name.empty()&&name[0]=='f') {
      //constants needed to compare with step.status:
      if (name=="fWorldBoundary") return create_constant((int_type)fWorldBoundary);
      if (name=="fGeomBoundary") return create_constant((int_type)fGeomBoundary);
      if (name=="fAtRestDoItProc") return create_constant((int_type)fAtRestDoItProc);
      if (name=="fAlongStepDoItProc") return create_constant((int_type)fAlongStepDoItProc);
      if (name=="fPostStepDoItProc") return create_constant((int_type)fPostStepDoItProc);
      if (name=="fUserDefinedLimit") return create_constant((int_type)fUserDefinedLimit);
      if (name=="fExclusivelyForcedProc") return create_constant((int_type)fExclusivelyForcedProc);
      if (name=="fUndefined") return create_constant((int_type)fUndefined);
      if (name=="kStateUndefined") return create_constant((int_type)kStateUndefined);
      if (name=="kStateSolid") return create_constant((int_type)kStateSolid);
      if (name=="kStateLiquid") return create_constant((int_type)kStateLiquid);
      if (name=="kStateGas") return create_constant((int_type)kStateGas);
      //nb: slightly faster, we could have: step.status_is_fWorldBoundary, etc.
    }

    //Finally, volatile value wrappers for data extracted from a G4Step
    //instance. Look for variables named trk.xxx, step.xxx, step.pre.xxx or
    //step.post.xxx and look for the correspondingly named xxx function in
    //G4DataExtractors.hh:

    std::vector<std::string> parts;
    Core::split_noempty(parts,name,".");

    const G4Step * & stepref = const_cast<G4SteppingASTBuilder*>(this)->m_currentStep;

    //Evil but convenient macro (can't stringify with pure C++):
#   ifdef TESTRETURN
#     undef TESTRETURN
#   endif
#   define TESTRETURN(x) if (p2==#x) { return wrap_extractor<extract_space::x>(name,stepref,is_poststep); }

    if ((parts.size()==2 || parts.size()==3)&& parts[0]=="step" && (parts[1]=="pre"||parts[1]=="post")) {
      if ( parts.size()!=3 )
        EXPRPARSER_THROW2(ParseError,"missing property of \"step."<<parts[1]<<"\" (specify like \"step."<<parts[1]<<".xxx\")");
      const bool is_poststep = (parts[1]=="post");
      namespace extract_space = DataExtractors::steppoint;
      auto& p2 = parts[2];
      switch(p2.empty()?'@':p2[0]) {
      case 'a':
        TESTRETURN(at_voledge); break;
      case 'b':
        TESTRETURN(beta); break;
      case 'e':
        TESTRETURN(ekin);
        TESTRETURN(etot);
        TESTRETURN(exists); break;
      case 'g':
        TESTRETURN(gamma);
        TESTRETURN(globaltime); break;
      case 'l':
        TESTRETURN(local_x);
        TESTRETURN(local_y);
        TESTRETURN(local_z);
        TESTRETURN(localtime); break;
      case 'm':
        TESTRETURN(mat_name);
        TESTRETURN(mat_state);
        TESTRETURN(mat_density);
        TESTRETURN(mat_temperature);
        TESTRETURN(mat_pressure);
        TESTRETURN(mom);
        TESTRETURN(mom2); break;
      case 'n':
        if (p2=="neutron_wl") {
          if (parts[1]=="pre")
            return wrap_extractor<DataExtractors::step::neutron_wl<true>>(name,stepref);
          else
            return wrap_extractor<DataExtractors::step::neutron_wl<false>>(name,stepref);
        }
        break;
      case 'p':
        TESTRETURN(px);
        TESTRETURN(py);
        TESTRETURN(pz);
        TESTRETURN(polx);
        TESTRETURN(poly);
        TESTRETURN(polz);
        TESTRETURN(process_defined_step);
        TESTRETURN(propertime); break;
      case 's':
        TESTRETURN(stepstatus); break;
      case 't':
        TESTRETURN(time); break;
      case 'v':
        TESTRETURN(velocity);
        TESTRETURN(volcopyno);
        TESTRETURN(volcopyno_1);
        TESTRETURN(volcopyno_2);
        TESTRETURN(volname);
        TESTRETURN(volname_1);
        TESTRETURN(volname_2); break;
      case 'w':
        TESTRETURN(weight); break;
      case 'x':
        TESTRETURN(x); break;
      case 'y':
        TESTRETURN(y); break;
      case 'z':
        TESTRETURN(z); break;
      }
      EXPRPARSER_THROW2(ParseError,"unknown property \""<<p2<<"\" of \"step."<<parts[1]<<"\"");
    }

    //adjust our evil macro:
#   undef TESTRETURN
#   define TESTRETURN(x) if (p2==#x) { return wrap_extractor<extract_space::x>(name,stepref); }

    if (parts.size()==2&&parts[0]=="step") {
      namespace extract_space = DataExtractors::step;
      auto& p2 = parts[1];
      switch(p2.empty()?'@':p2[0]) {
      case 'e':
        TESTRETURN(edep);
        TESTRETURN(edep_ion);
        TESTRETURN(edep_nonion); break;
      case 'd':
        TESTRETURN(delta_e);
        TESTRETURN(delta_mom);
        TESTRETURN(delta_pos);
        TESTRETURN(delta_time); break;
      case 'm':
        TESTRETURN(mat_name);
        TESTRETURN(mat_state);
        TESTRETURN(mat_density);
        TESTRETURN(mat_temperature);
        TESTRETURN(mat_pressure); break;
      case 'p':
        TESTRETURN(process_defined_step); break;
      case 's':
        TESTRETURN(steplength);
        TESTRETURN(stepnbr); break;
      case 'v':
        TESTRETURN(volcopyno);
        TESTRETURN(volcopyno_1);
        TESTRETURN(volcopyno_2);
        TESTRETURN(volname);
        TESTRETURN(volname_1);
        TESTRETURN(volname_2); break;
      }
      EXPRPARSER_THROW2(ParseError,"unknown step property : \""<<p2<<"\"");
    }

    if (parts.size()==2&&parts[0]=="trk") {
      namespace extract_space = DataExtractors::trk;
      auto& p2 = parts[1];
      switch(p2.empty()?'@':p2[0]) {
      case 'a':
        TESTRETURN(atomicmass);
        TESTRETURN(atomicnumber); break;
      case 'c':
        TESTRETURN(charge);
        TESTRETURN(creatorprocess); break;
      case 'i':
        TESTRETURN(is_gamma);
        TESTRETURN(is_ion);
        TESTRETURN(is_neutrino);;
        TESTRETURN(is_neutron);;
        TESTRETURN(is_opticalphoton);
        TESTRETURN(is_photon);
        TESTRETURN(is_primary);
        TESTRETURN(is_secondary);
        TESTRETURN(is_shortlived);
        TESTRETURN(is_stable); break;
      case 'l':
        TESTRETURN(lifetime); break;
      case 'm':
        TESTRETURN(magneticmoment);
        TESTRETURN(mass);
        TESTRETURN(mcplflag); break;
      case 'n':
        TESTRETURN(name); break;
      case 'p':
        TESTRETURN(parentid);
        TESTRETURN(pdgcode); break;
      case 's':
        TESTRETURN(spin);
        TESTRETURN(subtype); break;
      case 't':
        TESTRETURN(trkid);
        TESTRETURN(type); break;
      case 'w':
        TESTRETURN(weight);
        TESTRETURN(width); break;
      }
      EXPRPARSER_THROW2(ParseError,"unknown trk property : \""<<p2<<"\"");
    }

    if ( parts.size()==1 && (parts[0]=="step" || parts[0]=="trk") )
      EXPRPARSER_THROW2(ParseError,"missing property of \""<<parts[0]<<"\". specify like \""<<parts[0]<<".xxx\")");

    return 0;
  }

}
