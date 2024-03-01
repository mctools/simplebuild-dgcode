#define DOSUPPORT_NCRYSTALDEV_MATERIALS

#ifdef DOSUPPORT_NCRYSTALDEV_MATERIALS
#  include "PluginUtils/PluginHelper.hh"//for dynamic NCrystalDev support
#endif
#include "G4Materials/NamedMaterialProvider.hh"
#include "G4Materials/CommonMaterials.hh"
#include "G4Materials/ShieldingMaterials.hh"
#include "G4NCrystalRel/G4NCrystal.hh"
#include "NCrystal/internal/NCCfgTypes.hh"
#include "NCrystal/internal/NCMath.hh"
#include "NCrystal/NCFactImpl.hh"
#include "IdealGasBuilder/IdealGasBuilder.hh"
#include "Units/Units.hh"
#include "Core/FindData.hh"
#include "Core/String.hh"
#include "Core/File.hh"
#include <cstdlib>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>

namespace G4NC = G4NCrystalRel;
namespace NC = NCrystal;

namespace NamedMaterialProvider {

#ifdef DOSUPPORT_NCRYSTALDEV_MATERIALS
  namespace {
    const G4Material * loadNCrystalDevMaterial( const std::string& cfgstr )
    {
      static auto thelib = PluginHelper::loadPkgLib( "NCrystalPreview" );
      static auto thefunc =
        thelib->getFunction<G4Material*(const char*)>
        ("plugin_ncrystaldev_create_material");
      return thefunc( cfgstr.c_str() );
    }
  }
#endif

  static std::string s_prefix;
  void setPrintPrefix(const char* p)
  {
    s_prefix = p;
    s_prefix += "NamedMaterial ";
  }

  struct MatInfo {
    MatInfo(const std::string&fullname);
    std::string fullname;
    std::string name;
    std::map<std::string,std::string> properties;

    bool hasProperty(const std::string&p) const { return properties.find(p)!=properties.end(); }
    bool propertyAsBool(const std::string&p, bool default_value) const {
      auto it = properties.find(p);
      if (it==properties.end())
        return default_value;
      std::string valstr = it->second;
      Core::to_lower(valstr);
      if (valstr=="1"||valstr=="yes"||valstr=="y"||valstr=="on"||valstr=="true")
        return true;
      else if (valstr=="0"||valstr=="no"||valstr=="n"||valstr=="off"||valstr=="false")
        return false;
      bad("Could not convert property to boolean value");
      return false;
    }
    double propertyAsDouble(const std::string&p, double default_value) const {
      auto it = properties.find(p);
      if (it==properties.end())
        return default_value;
      double dbl = 0.0;
      std::istringstream num(it->second);
      num >> dbl;
      if(num.fail() || !num.eof()) {
        bad("Could not convert property to float");
      }
      return dbl;
    }

    std::string propertyAsString(const std::string&p, const std::string& default_value) const {
      auto it = properties.find(p);
      if (it==properties.end())
        return default_value;
      return it->second;
    }

    bool hasNonDefaultDensity() const {
      return hasProperty("scale_density")||hasProperty("density_gcm3")||hasProperty("density_kgm3");
    }

    double getDensity(double default_density) const {
      if (hasProperty("scale_density"))
        return default_density * propertyAsDouble("scale_density",1.0);
      if (hasProperty("density_gcm3"))
        return propertyAsDouble("density_gcm3",default_density/(Units::gram/Units::cm3))*(Units::gram/Units::cm3);
      if (hasProperty("density_kgm3"))
        return propertyAsDouble("density_kgm3",default_density/(Units::kilogram/Units::m3))*(Units::kilogram/Units::m3);
      return default_density;
    }

    std::string getDensityAsNCrystalCfgStrPostfix() const {
      if (hasProperty("scale_density")) {
        std::ostringstream os;
        os << ";density="<<NC::fmt( propertyAsDouble("scale_density",-999.0) ) << "x";
        return os.str();
      } else if (hasProperty("density_gcm3")) {
        std::ostringstream os;
        os << ";density="<<NC::fmt( propertyAsDouble("density_gcm3", -999.0) ) << "gcm3";
        return os.str();
      } else if (hasProperty("density_kgm3")) {
        std::ostringstream os;
        os << ";density="<<NC::fmt( propertyAsDouble("density_kgm3",-999.0) ) << "kgm3";
        return os.str();
      }
      return std::string();
    }

    std::string getTempAsNCrystalCfgStrPostfix() const {
      if (hasProperty("temp_kelvin")) {
        std::ostringstream ss;
        ss << ";temp="<<NC::fmt( propertyAsDouble("temp_kelvin",-999.0) ) << "K";
        return ss.str();
      }
      return std::string();
    }

    void bad(const char*c) const {
      printf("%sERROR in material \"%s\": %s\n",s_prefix.c_str(),fullname.c_str(),c);
      exit(1);
    }
  };
}

NamedMaterialProvider::MatInfo::MatInfo(const std::string& ss)
  : fullname(ss)
{
  //Decode string according to the format:
  //
  //   "name:parameter1=value1:parameter2=value2:..."
  //
  //'=' and ':' characters are forbidden in name and parameter names.
  //
  //Values can optionally be enclosed in [], in which case it can contain
  //'=' and ':' characters, otherwise those are forbidden in values.

  std::string par,val;
  size_t p = ss.find(':');
  if (p==std::string::npos) {
    name = ss;
  } else {
    name.assign(ss,0,p);
    while (true) {
      //Roll off any leading ':', while constantly pre-checking for end of string (and aborting if found)
      while (p<ss.size()&&ss.at(p)==':')
        ++p;
      if (p>=ss.size())
        break;//done
      //decode parameter name:
      size_t p2 = ss.find('=',p);
      if (p2==p||p2==std::string::npos)
        bad("syntax error in parameter definition");
      assert(p2>p);
      par.assign(ss,p,p2-p);
      p = p2 + 1;
      //Get value, either by looking for the next divider (':') or the end of
      //the string, or by eating everything enclosed in square brackets ('[]'):
      if (p<ss.size()&&ss.at(p)=='[') {
        p2 = ss.find_first_of(']',p);
        if (p2==std::string::npos)
          bad("closing square bracket missing in parameter definition");
        assert(p2>p+1);
        val.assign(ss,p+1,p2-p-1);
        p = p2+1;
      } else {
        p2 = ss.find(':',p);
        if (p2==std::string::npos) {
          val.assign(ss.c_str()+p);
          p=ss.size();
        } else {
          assert(p2>p);
          val.assign(ss,p,p2-p);
          p=p2;
          ++p;
        }
      }
      properties[par]=val;
    }
  }
}

//FIXME: return const materials
G4Material * NamedMaterialProvider::getMaterial(const std::string& sss)
{
  constexpr double stp_temp = 273.15 * Units::kelvin;
  constexpr double stp_pressure = 1.0 * Units::atm;

  std::string ss = NC::trim2( sss );

  auto validate = []( const G4Material * mat)
  {
    if ( !mat )
      throw std::runtime_error("NamedMaterialProvider ERROR: Would"
                               " return nullptr");
    //Guard against: https://bugzilla-geant4.kek.jp/show_bug.cgi?id=2520:
    if ( mat->GetBaseMaterial() && mat->GetBaseMaterial()->GetBaseMaterial() )
      throw std::runtime_error("NamedMaterialProvider ERROR: Would return"
                               " double-based material which is not supported");
    return const_cast<G4Material*>(mat); //FIXME!
  };

  //Map aliases (only on exact string contents, no other parameters allowed to
  //keep support trivial):

  if ( ss.size() < 12 && ( ( ss.back() == 'm'
                             && NC::isOneOf( ss, "Vacuum", "G4_Vacuum" ) )
                           || ss == "G4_Galactic" ) )
    return CommonMaterials::getMaterial_Vacuum();

  if ( ss.size() < 8 && ( NC::startswith(ss,'E') || NC::startswith(ss,'M') ) ) {
    if (NC::isOneOf(ss,"ESS_Al","MAT_Al"))
      return validate( G4NC::createMaterial("stdlib::Al_sg225.ncmat") );
    if (NC::isOneOf(ss,"ESS_Cu","MAT_Cu"))
      return validate( G4NC::createMaterial("stdlib::Cu_sg225.ncmat") );
    if (NC::isOneOf(ss,"ESS_Ti","MAT_Ti"))
      return validate( G4NC::createMaterial("stdlib::Ti_sg194.ncmat") );
    if (NC::isOneOf(ss,"ESS_V","mat_V"))
      return validate( G4NC::createMaterial("stdlib::V_sg229.ncmat") );
  }

  //Support pure NCrystal cfg strings:
  {
    auto ncrystal_matcfg = [&ss]() -> NC::Optional<NC::MatCfg>
      {
        auto sv = NC::StrView(ss);
        if ( sv.contains_any(NC::Cfg::forbidden_chars_multiphase ) )
          return NC::NullOpt;
        if ( !sv.contains_any(".;:</" ) )
          return NC::NullOpt;
        const static std::vector<std::string> reserved_words = {
          "IdealGas","NCrystal","NCrystalDev", "MIX",
          "MAT_B4C","MAT_POLYETHYLENE",
          "ESS_B4C","ESS_POLYETHYLENE"
        };
        for ( auto& rw : reserved_words ) {
          if ( !NC::startswith(ss,rw) )
            continue;
          auto svtrail = NC::StrView(ss).substr( rw.size() ).ltrimmed();
          if ( svtrail.empty() || NC::isOneOf( svtrail[0], ';', ':' ) )
            return NC::NullOpt;
        }

        auto parts = sv.splitTrimmedNoEmpty(':');
        if ( parts.empty() )
          return NC::NullOpt;
        auto& p0 = parts.at(0);
        // for (const auto& reserved_word : { "IdealGas","NCrystal","NCrystalDev", "MIX",
        //                  "MAT_B4C","MAT_POLYETHYLENE",
        //                  "ESS_B4C","ESS_POLYETHYLENE" }
        // if ( NC::isOneOf(p0,
        //                  "IdealGas","NCrystal","NCrystalDev", "MIX",
        //                  "MAT_B4C","MAT_POLYETHYLENE",
        //                  "ESS_B4C","ESS_POLYETHYLENE" ) )
        //   return NC::NullOpt;
        if ( !p0.contains_any(".</")
             && ( p0.startswith( "G4_" ) || p0.startswith( "SHIELDING_" ) ) )
          return NC::NullOpt;
        NC::Optional<NC::MatCfg> res;
        try {
          NC::MatCfg trycfg( ss );
          res = std::move(trycfg);
        } catch ( NC::Error::BadInput& ) {
        }
        return res;
      }();

    if ( ncrystal_matcfg.has_value() ) {
      //No caching, just rely on NCrystal:
      return validate( G4NC::createMaterial( ncrystal_matcfg.value() ) );
    }
  }

  //check cache:
  static std::map<std::string,G4Material*> cache;
  auto itCache = cache.find(ss);
  if (itCache!=cache.end()) {
    return itCache->second;
  }

  //Decode:
  MatInfo matinfo(ss);

  ////////////////////////////////////////////////////////
  ////// Check that only allowed properties are set //////
  ////////////////////////////////////////////////////////

  std::set<std::string> allowedproperties;
  allowedproperties.insert("temp_kelvin");
  allowedproperties.insert("scale_density");
  allowedproperties.insert("density_gcm3");
  allowedproperties.insert("density_kgm3");
  if (matinfo.name=="IdealGas") {
    allowedproperties.insert("pressure_bar");
    allowedproperties.insert("pressure_atm");
    allowedproperties.insert("formula");
  } else if (NC::isOneOf(matinfo.name,"MAT_POLYETHYLENE","ESS_POLYETHYLENE")) {
    //nothing special (should anyway end up as TS_POLYETHYLENE (and we should have option to create TS mats from G4 nist mats
  } else if (matinfo.name=="NCrystalDev") {
    allowedproperties.insert("cfg");
  } else if (matinfo.name=="NCrystal") {
    allowedproperties.insert("cfg");
    allowedproperties.insert("overridebaseg4mat");//can specify G4 NIST material to combine with NC::Scatter
    allowedproperties.insert("g4physicsonly");//No actual physics from NCrystal
  } else if (matinfo.name=="MIX") {//
    allowedproperties.insert("comp1");
    allowedproperties.insert("comp2");
    allowedproperties.insert("comp3");
    allowedproperties.insert("comp4");
    allowedproperties.insert("f1");
    allowedproperties.insert("f2");
    allowedproperties.insert("f3");
    allowedproperties.insert("f4");
    allowedproperties.insert("allowstatemix");
  } else if (NC::isOneOf(matinfo.name,"MAT_B4C","ESS_B4C")) {
    allowedproperties.insert("b10_enrichment");
  }
  for (auto itP=matinfo.properties.begin();itP!=matinfo.properties.end();++itP) {
    if (!allowedproperties.count(itP->first)) {
      printf("%sERROR: Unknown property for material \"%s\": \"%s\"\n",s_prefix.c_str(),matinfo.name.c_str(),itP->first.c_str());
      exit(1);
    }
  }
  int ndensitypars =    ( matinfo.hasProperty("scale_density") ? 1 : 0 )
                      + ( matinfo.hasProperty("density_gcm3")  ? 1 : 0 )
                      + ( matinfo.hasProperty("density_kgm3")  ? 1 : 0 );
  if (ndensitypars>1) {
    printf("%sERROR: Material \"%s\" has more than one of \"scale_density\", \"density_gcm3\" and \"density_kgm3\" set\n",
           s_prefix.c_str(),matinfo.name.c_str());
    exit(1);
  }

  //Create material:
  G4Material * mat(0);
  if (matinfo.name=="IdealGas") {
    //Decode formula
    const std::string formula = matinfo.propertyAsString("formula","");
    if (formula.empty()) {
      printf("%sERROR: Material \"%s\" must have parameters \"formula\" set.\n",
             s_prefix.c_str(),matinfo.name.c_str());
      exit(1);
    }

    //Decode density:
    if (matinfo.hasProperty("scale_density")) {
      printf("%sERROR: scale_density parameter is not supported for %s'es.\n",
             s_prefix.c_str(),matinfo.name.c_str());
      exit(1);
    }
    bool fixedDensity = matinfo.hasProperty("density_gcm3") || matinfo.hasProperty("density_kgm3");
    double density = fixedDensity ? matinfo.getDensity(0.0) : 0;

    //Decode pressure:
    double pressure = stp_pressure;
    bool fixedPressure = matinfo.hasProperty("pressure_bar") || matinfo.hasProperty("pressure_atm");
    if (fixedPressure) {
      if (matinfo.hasProperty("pressure_bar")) {
        if (matinfo.hasProperty("pressure_atm")) {
          printf("%sERROR: Material \"%s\" has both \"pressure_bar\" and \"pressure_atm\" set.\n",
                 s_prefix.c_str(),matinfo.name.c_str());
          exit(1);
        }
        pressure = matinfo.propertyAsDouble("pressure_bar",pressure) * Units::bar;
      } else {
        assert(matinfo.hasProperty("pressure_atm"));
        pressure = matinfo.propertyAsDouble("pressure_atm",pressure) * Units::atmosphere;
      }
    }

    //Decode temperature:
    bool fixedTemperature = matinfo.hasProperty("temp_kelvin");
    double temperature = matinfo.propertyAsDouble("temp_kelvin",stp_temp/Units::kelvin)*Units::kelvin;
    unsigned nfixed = (fixedTemperature?1:0)+(fixedPressure?1:0)+(fixedDensity?1:0);
    if (nfixed==0) {
      fixedTemperature = fixedPressure = true;
      temperature = stp_temp;
      pressure = stp_pressure;
      nfixed += 2;
    }
    if (nfixed==3) {
      printf("%sERROR: Material \"%s\" is overspecified. Supply at most two of temperature, pressure and density.\n",
             s_prefix.c_str(),matinfo.name.c_str());
      exit(1);
    }
    if (nfixed==1) {
      if (!fixedTemperature) {
        fixedTemperature = true;
        temperature = stp_temp;
        nfixed += 1;
      } else {
        assert(!fixedPressure);
        fixedPressure = true;
        pressure = stp_pressure;
        nfixed += 1;
      }
    }
    assert (nfixed==2);
    if (fixedTemperature&&fixedPressure) {
      mat = IdealGas::createMaterialCalcD(formula.c_str(), temperature, pressure );
    } else if (fixedTemperature&&fixedDensity) {
      mat = IdealGas::createMaterialCalcP(formula.c_str(), density, temperature);
    } else {
      assert(fixedPressure&&fixedDensity);
      mat = IdealGas::createMaterialCalcT(formula.c_str(), density, pressure);
    }
  } else if (NC::isOneOf(matinfo.name,"MAT_POLYETHYLENE","ESS_POLYETHYLENE")) {
    G4Material * mat_g4pe = CommonMaterials::getNISTMaterial("G4_POLYETHYLENE",s_prefix.c_str());
    const double temperature = matinfo.propertyAsDouble("temp_kelvin",stp_temp/Units::kelvin)*Units::kelvin;
    const double density = matinfo.getDensity(mat_g4pe->GetDensity());

    //Polyethylene (copied from XX's code - this is for temporary testing only!)
    G4Isotope *isoH_PE=new G4Isotope("isoH_poly", 1, 1, 1.0079 *CLHEP::g/CLHEP::mole);
    G4Element* elTSH = new G4Element("TS_H_of_Polyethylene", "TS_POLYETHYLENE", 1);
    elTSH->AddIsotope(isoH_PE,1.);

    G4Element* elementC = CommonMaterials::getNISTElement("C",s_prefix.c_str());
    //G4Element* elementC = new G4Element( "Carbon", "C", 6. , 12.011*CLHEP::g/CLHEP::mole );

    G4Material* matCH2_TS = new G4Material("Polyethylene_TS", density, 2, kStateSolid, temperature);
    matCH2_TS -> AddElement(elTSH,2);
    matCH2_TS -> AddElement(elementC,1);

    // G4NeutronAceTSManager *tsman= G4NeutronAceTSManager::GetInstance();
    // tsman->RegisterData(elTSH->GetIsotope(0),"hch2_acer.h5","HCH2.71t");
    // tsman->RegisterData(elTSH->GetIsotope(0),"CH2-293_ace.h5","pol00.32t");

    mat = matCH2_TS;
    mat->SetChemicalFormula(mat_g4pe->GetChemicalFormula());
    mat->GetIonisation()->SetMeanExcitationEnergy(mat_g4pe->GetIonisation()->GetMeanExcitationEnergy());

    // std::map<std::pair<int,int>, std::string> isotope_2_mat;
    // isotope_2_mat[std::make_pair(1,1)] = "bla";
    // G4Material * mat_orig = CommonMaterials::getNISTMaterial("G4_POLYETHYLENE",s_prefix.c_str());//base material
    // const double density = matinfo.getDensity(mat_orig->GetDensity());
    // auto state = mat_orig->GetState();
    // const double pressure = mat_orig->GetPressure();//or stp_pressure?
    // auto elemV = mat_orig->GetElementVector();
    // auto atomV = mat_orig->GetAtomsVector();
    // mat = new G4Material(matinfo.fullname.c_str(),density,
    //                      elemV->size(),
    //                      state,temperature,pressure);
    // //mat->SetMassOfMolecule(mat_orig->GetMassOfMolecule());
    // for (unsigned i=0;i<elemV->size();++i) {
    //   G4Element * elem_orig = elemV->at(i);
    //   GetIsotopeVector * isoV = elem_orig->GetIsotopeVector();
    //   G4Element * elem = new G4Element(const G4String& name,
    //                                    const G4String& symbol,
    //                                    G4int nbIsotopes);

    //   mat->AddElement(elem_orig,atomV[i]);//FIXME: Recreate!!!
    // }
    // mat->SetChemicalFormula(mat_orig->GetChemicalFormula());
    // mat->GetIonisation()->SetMeanExcitationEnergy(mat_orig->GetIonisation()->GetMeanExcitationEnergy());

  } else if (matinfo.name=="NCrystalDev") {
#ifdef DOSUPPORT_NCRYSTALDEV_MATERIALS
    std::string cfgstr = matinfo.propertyAsString("cfg","");
    cfgstr += matinfo.getTempAsNCrystalCfgStrPostfix();
    cfgstr += matinfo.getDensityAsNCrystalCfgStrPostfix();
    //Use NCrystal from the NCrystalPreview package in the ncrystaldev repo, but
    //use dynamic bindings to avoid a static dependency (FIXME: It would be
    //better to enable dynamic "NamedMaterial factories" and have such a factory
    //in the NCrystalDev repo!!!).
    {
      const char * essinstdir = getenv("SBLD_INSTALL_PREFIX");
      if ( !essinstdir || !Core::file_exists(std::string(essinstdir)+"/python/NCrystalPreview/__init__.py") ) {
        throw std::runtime_error("ERROR: In order to use \"NCrystalDev\" materials the NCrystalPreview package\n"
                                 "must be enabled. This is intended solely for code in the ncrystaldev repo, most\n"
                                 "users should use \"NCrystal\" instead of \"NCrystalDev\".\n");
      }
      const G4Material * ncdevmat = loadNCrystalDevMaterial(cfgstr);
      if (!ncdevmat)
        throw std::runtime_error("Got null G4Material pointer"
                                 " for NCrystalDev material.");
      mat = const_cast<G4Material *>(ncdevmat);//fixme: https://github.com/mctools/simplebuild-dgcode/issues/34
    }
#else
    throw std::runtime_error("NamedMaterialProvider was compiled without support for NCrystalDev materials");
#endif
  } else if (matinfo.name=="NCrystal") {
    const bool opt_g4physicsonly = matinfo.propertyAsBool("g4physicsonly",false);
    const std::string& overridebaseg4mat = matinfo.propertyAsString("overridebaseg4mat","");
    if ( opt_g4physicsonly && !overridebaseg4mat.empty() )
      throw std::runtime_error("Do not specify both \"g4physicsonly\" and \"overridebaseg4mat\"");
    std::string cfgstr = matinfo.propertyAsString("cfg","");
    //Absorp density/temp parameters into cfgstr (this also avoids nesting G4
    //base-materials which is apparently not supported).
    cfgstr += matinfo.getTempAsNCrystalCfgStrPostfix();
    cfgstr += matinfo.getDensityAsNCrystalCfgStrPostfix();
    auto ncrystal_matcfg = NC::MatCfg( cfgstr );
    if (overridebaseg4mat.empty()) {
      mat = G4NC::createMaterial( ncrystal_matcfg );
      if ( opt_g4physicsonly ) {
        const G4Material * matbase = mat->GetBaseMaterial();
        if ( !matbase )
          throw std::runtime_error("NCrystal material does not have base material.");
        std::string new_name = mat->GetName();
        new_name += "_NoNCrystalPhysics";
        mat = new G4Material( new_name,
                              mat->GetDensity(),
                              matbase,
                              mat->GetState(),
                              mat->GetTemperature(),
                              mat->GetPressure() );
      }
    } else {
      mat = CommonMaterials::getNISTMaterial(overridebaseg4mat.c_str(),s_prefix.c_str());//base material selected directly by user
      auto ncsc = NC::FactImpl::createScatter( ncrystal_matcfg );
      auto ncinfo = NC::FactImpl::createInfo( ncrystal_matcfg );
      const double density = matinfo.getDensity( mat->GetDensity() );//We apparently take the density+state from the NIST base material,
                                                                     //but not temperature (not sure if this is the best way?)
      mat = new G4Material(matinfo.fullname.c_str(),
                           density,
                           mat,
                           mat->GetState(),
                           ncinfo->getTemperature().dbl(),
                           mat->GetPressure());
      G4NC::Manager::getInstance()->addScatterProperty(mat,std::move(ncsc));
    }
  } else if (NC::isOneOf(matinfo.name,"MAT_B4C","ESS_B4C")) {

    //////////////////////////////////////////////////////////////////////////
    ////// Boron Carbide - possibly changed density/temp and enrichment //////
    //////////////////////////////////////////////////////////////////////////

    double fraction_b10;
    if (!matinfo.hasProperty("b10_enrichment")) {
      fraction_b10 = CommonMaterials::getNaturalB10IsotopeFraction();
    } else {
      fraction_b10 = matinfo.propertyAsDouble("b10_enrichment",-1.0);
      if ( ! (fraction_b10>=0.0 && fraction_b10<=1.0) )
        matinfo.bad("b10_enrichment must be between 0.0 and1.0");
    }

    const double density = matinfo.getDensity(CommonMaterials::getDensity_BoronCarbide(fraction_b10));
    const double temperature = matinfo.propertyAsDouble("temp_kelvin",stp_temp)*Units::kelvin;
    mat = CommonMaterials::getMaterial_BoronCarbide(fraction_b10,density,temperature);

  } else if (matinfo.name=="MIX") {//
    const std::string str_empty;
    const std::string& comp1 = matinfo.propertyAsString("comp1",str_empty);
    const std::string& comp2 = matinfo.propertyAsString("comp2",str_empty);
    const std::string& comp3 = matinfo.propertyAsString("comp3",str_empty);
    const std::string& comp4 = matinfo.propertyAsString("comp4",str_empty);
    const double f1 = matinfo.propertyAsDouble("f1",0.0);
    const double f2 = matinfo.propertyAsDouble("f2",0.0);
    const double f3 = matinfo.propertyAsDouble("f3",0.0);
    const double f4 = matinfo.propertyAsDouble("f4",0.0);
    const bool can_mix_state = matinfo.propertyAsBool("allowstatemix",false);

    //check sanity of input:
    if (f1<0.0||f1>1.0||f2<0.0||f2>1.0||f3<0.0||f3>1.0||f4<0.0||f4>1.0) matinfo.bad("fractions must be between 0.0 and 1.0");
    if (fabs(f1+f2+f3+f4-1.0)>1.0e-10) matinfo.bad("sum of fractions must be 1.0");
    if ((f1<1.0e-10?true:false)!=comp1.empty()
        ||(f2<1.0e-10?true:false)!=comp2.empty()
        ||(f3<1.0e-10?true:false)!=comp3.empty()
        ||(f4<1.0e-10?true:false)!=comp4.empty())
      matinfo.bad("a fraction must be > 0.0 if and only if a corresponding component name is specified");

    //Simply use G4Material::AddMaterial to create a material mix.

    std::vector<std::pair<G4Material*,double> > components;

    if (!comp1.empty()) components.push_back(std::make_pair(getMaterial(comp1.c_str()),f1));
    if (!comp2.empty()) components.push_back(std::make_pair(getMaterial(comp2.c_str()),f2));
    if (!comp3.empty()) components.push_back(std::make_pair(getMaterial(comp3.c_str()),f3));
    if (!comp4.empty()) components.push_back(std::make_pair(getMaterial(comp4.c_str()),f4));
    auto itE = components.end();

    std::set<G4State> states;
    for (auto it=components.begin();it!=itE;++it) {
      states.insert(it->first->GetState());
      if (can_mix_state)
        break;//just query the first provided component from state and ignore the rest
    }
    if (states.size()>1)
      matinfo.bad("specified components must be in the same solid/liquid/gas state");

    //this density combination is a bit dubious at best, but the user can override the density as needed.
    double density(0);
    for (auto it=components.begin();it!=itE;++it)
      density += it->first->GetDensity() * it->second;

    if (!matinfo.hasNonDefaultDensity()) {
      if (density < 0.1*Units::g/Units::cm3)
        printf("%sINFO: Density of requested material \"%s\" is: %g kg/m3\n",
               s_prefix.c_str(),matinfo.name.c_str(),density/((1000.0*Units::gram)/Units::m3));
      else
        printf("%sINFO: Density of requested material \"%s\" is: %g g/cm3\n",
               s_prefix.c_str(),matinfo.name.c_str(),density/(Units::gram/Units::cm3));
    }

    const double temperature = matinfo.propertyAsDouble("temp_kelvin",stp_temp/Units::kelvin)*Units::kelvin;

    mat = new G4Material(matinfo.fullname,
                         matinfo.getDensity(density),
                         components.size(),
                         *(states.begin()),
                         temperature);

    for (auto it=components.begin();it!=itE;++it)
      mat->AddMaterial(it->first,it->second);

  } else {

    /////////////////////////////////////////////////////////////
    ////// NIST materials, NCrystal or Shielding MATERIALS //////
    ////// - POSSIBLY WITH CHANGED DENSITY AND TEMPERATURE //////
    /////////////////////////////////////////////////////////////

    if ( NC::contains(matinfo.name, ';') ) {
      printf("%sERROR: Invalid request (did you use \";\" "
             "instead of \":\" ?): \"%s\".\n",
             s_prefix.c_str(),matinfo.name.c_str());
      exit(1);
    }

    if ( NC::contains_any(matinfo.name, ";:<>/[]\\" ) ) {
      printf("%sERROR: Unexpected special character in requested"
             " material name \"%s\"\n",
             s_prefix.c_str(),matinfo.name.c_str());
      exit(1);
    }

    if (Core::starts_with(matinfo.name,"SHIELDING_")) {
      //Shielding material
      mat = ShieldingMaterials::createMat(matinfo.name);
      if (!mat) {
        printf("%sERROR: Unknown shielding material \"%s\"\n",
               s_prefix.c_str(),matinfo.name.c_str());
        exit(1);
      }
    } else {
      //G4 nist material
      mat = CommonMaterials::getNISTMaterial(matinfo.name.c_str(),
                                             s_prefix.c_str());
      if (!mat) {
        printf("%sERROR: Unknown material \"%s\"\n",
               s_prefix.c_str(),matinfo.name.c_str());
        exit(1);
      }
    }
    if (matinfo.hasNonDefaultDensity()||matinfo.hasProperty("temp_kelvin")) {
      //wrap the pure nist material in a material with changed density and/or temp:
      const double temperature = matinfo.propertyAsDouble("temp_kelvin",stp_temp/Units::kelvin)*Units::kelvin;
      G4Material * mat0 = mat;
      mat = new G4Material(matinfo.fullname.c_str(),
                           matinfo.getDensity(mat0->GetDensity()),
                           mat0,
                           mat0->GetState(),
                           temperature,
                           mat0->GetPressure());
    } else {
      assert(matinfo.name==matinfo.fullname);//no other properties
    }
  }

  assert(mat);

  //make sure the name is set consistently (only when material does not already
  //have a base material!!!)  See also
  //https://bugzilla-geant4.kek.jp/show_bug.cgi?id=2520

  if ( matinfo.fullname != mat->GetName() && !mat->GetBaseMaterial() ) {
    G4Material * mat0 = mat;
    mat = new G4Material(matinfo.fullname.c_str(),
                         mat0->GetDensity(),
                         mat0,
                         mat0->GetState(),
                         mat0->GetTemperature(),
                         mat0->GetPressure());
    //NB: No need to copy NCrystal property, since it resides in the
    //G4MaterialPropertyTable which is copied over when used as a base material.
    //update: the mat->GetBaseMaterial() check means we never do this for the
    //NCrystal materials
    //assert(matinfo.fullname == mat->GetName());
  }

  cache[ss] = mat;
  return validate( mat );
}
