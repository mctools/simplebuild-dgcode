#include "G4ExprParser/G4SteppingASTBuilder.hh"
#include "ExprParser/ASTDebug.hh"
#include "G4Launcher/Launcher.hh"
#include "G4Interfaces/GeoConstructBase.hh"
#include "Units/Units.hh"

#include "G4NistManager.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4Box.hh"
#include "G4UserSteppingAction.hh"
#include <functional>

namespace {
  class DummyGeo final : public G4Interfaces::GeoConstructBase {
  public:

    DummyGeo() : GeoConstructBase("DummyGeo")
    {
      addParameterDouble("boxThickness",1*Units::um,0.01*Units::um,300*Units::cm);
    }

    G4VPhysicalVolume* Construct() override
    {
      G4Material* mat_galactic = G4NistManager::Instance()->FindOrBuildMaterial("G4_Galactic",true);
      G4Material* mat_boroncarbide = G4NistManager::Instance()->FindOrBuildMaterial("G4_BORON_CARBIDE",true);
      G4Box* solidWorld = new G4Box("World", 10*Units::meter,10*Units::meter,10*Units::meter);
      G4LogicalVolume* logicWorld = new G4LogicalVolume(solidWorld, mat_galactic, "World");
      auto pv_world = new G4PVPlacement(0, G4ThreeVector(0.0, 0.0, 0.0), logicWorld, "World", 0, false, 0);
      G4Box* solidBox = new G4Box("SldSomeBox", 5*Units::meter,5*Units::meter,getParameterDouble("boxThickness")*0.5);
      auto logicBox= new G4LogicalVolume(solidBox, mat_boroncarbide, "LVSomeBox");
      new G4PVPlacement(0, G4ThreeVector(0.0,0.0,0.0),"PVSomeBox", logicBox, pv_world, false, 0, true);

      return pv_world;
    }
  };

  std::function<bool(const G4Step*)> createG4StepFilter_opt1(const char * expr)
  {
    //Test implementation of how to create a self-contained filter function based
    //on an expression containing G4Step related variables, including proper
    //lifetime management of instantiated objects (leaving it here, just for a
    //compilation check):

    //Using shared rather than unique ptr here, since we can't capture-by-move until c++14:
    auto builder = std::make_shared<G4ExprParser::G4SteppingASTBuilder>();
    auto evaluator = builder->createEvaluator<bool>(expr);
    return [builder,evaluator](const G4Step* step) mutable->bool { builder->setCurrentStep(step); return evaluator(); };
  }

  class CustomStepAct final : public G4UserSteppingAction {

    G4ExprParser::G4SteppingASTBuilder m_builder;
    ExprParser::Evaluator<ExprParser::float_type> m_eval_value;
    ExprParser::Evaluator<bool> m_eval_filter;
  public:

    CustomStepAct(const char * filter_expr, const char * value_expr) : G4UserSteppingAction() {
      m_eval_filter = m_builder.createEvaluator<bool>(filter_expr);
      m_eval_value = m_builder.createEvaluator<ExprParser::float_type>(value_expr);
      printf("-------------------------------------------------------\n");
      ExprParser::printTree(m_eval_filter.arg(),"    |",false);
      printf("-------------------------------------------------------\n");
      ExprParser::printTree(m_eval_value.arg(),"    |",false);
      printf("-------------------------------------------------------\n");
    }
    ~CustomStepAct(){}

    void UserSteppingAction( const G4Step* step ) override {
      m_builder.setCurrentStep(step);//Must set before attempting to evaluate filter or value!
      printf("Test - filter: %i\n",(int)m_eval_filter());
      printf("Test - value : %g\n",m_eval_value());
    }

  };
}

int main(int,char**) {

  (void)createG4StepFilter_opt1;

  DummyGeo * geo = new DummyGeo();
  geo->setParameterDouble("boxThickness",20*Units::cm);

  G4Launcher::Launcher launcher;
  launcher.setGeo(geo);
  launcher.setParticleGun(2112, 0.025*Units::eV, G4ThreeVector(0,0,-5*Units::meter), G4ThreeVector(0,0,1));
  launcher.setOutput("none");
  launcher.setPhysicsList("PL_Empty");
  launcher.cmd_postinit("/tracking/verbose 1");
  launcher.init();
  launcher.setUserSteppingAction(new CustomStepAct("trk.trkid==1 && step.stepnbr > 2",
                                                   "step.steplength/cm + pi*sin(step.edep/MeV) + 1e9*step.post.volcopyno/keV + 5*(step.post.volname_2=='World')"));
  launcher.startSimulation(1);
}
