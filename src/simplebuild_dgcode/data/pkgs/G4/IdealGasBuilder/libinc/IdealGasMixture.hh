#ifndef IdealGasBuilder_IdealGasMixture_hh
#define IdealGasBuilder_IdealGasMixture_hh

//Mixture class. See IdealGasBuilder.hh for a description of the purpose of this
//class, as well as the syntax for the "formula" arguments below.
//
//Author: Thomas Kittelmann, January 2015.

#include <string>
#include <vector>
#include <map>
#include <utility>
class G4Material;
class G4Element;

namespace IdealGas {

  class Component;

  class Mixture {
  public:

    ////////////////////////////////////////////////////////////////////////
    //                            Set up Mixture                          //
    ////////////////////////////////////////////////////////////////////////

    //Use this constructor to quickly create a mixture using a mixture formula
    //(see IdealGasBuilder.hh section 2.3):
    Mixture(const char * formula);

    //Empty constructor. When using this, you must subsequently define the
    //mixture with addComponent/addComponents calls:
    Mixture();

    //Add a component and an associated fraction (by volume or mole). Note that
    //the class will keep a pointer to the component, but will not assume
    //ownership of it unless manage = true:
    void addComponent( Component*, double fraction, bool manage = false );
    void addComponent( Component&, double fraction, bool manage = false );

    //Same, but with fractions by-mass:
    void addComponentFractionByMass( Component*, double fraction, bool manage = false );
    void addComponentFractionByMass( Component&, double fraction, bool manage = false );

    //Convenience version simply specifying the component formula (see IdealGasBuilder.hh section 2.2):
    void addComponent(const char* formula,double fraction);
    void addComponentFractionByMass(const char* formula,double fraction);

    //Add multiple components at once using a mixture formula (see
    //IdealGasBuilder.hh section 2.3):
    void addComponents(const char* formula);

    ////////////////////////////////////////////////////////////////////////
    //  Use mixture to generate G4 materials                              //
    ////////////////////////////////////////////////////////////////////////

    // Create G4 material representing the gas (see IdealGasBuilder.hh section 3.0):
    G4Material * createMaterialCalcD(double temp = -1, double pressure = -1, const char * name = 0);//Density will be calculated
    G4Material * createMaterialCalcP(double density, double temp = -1, const char * name = 0);//Pressure will be calculated
    G4Material * createMaterialCalcT(double density, double pressure = -1, const char * name = 0);//Temperature will be calculated
    G4Material * createMaterial(double temp = -1, double pressure = -1, const char * name = 0);//Same as createMaterialCalcD

    //Destructor:
    ~Mixture();

  private:
    std::vector<std::pair<Component*,double> > m_components;
    std::vector<Component*> m_owned_components;
    bool m_by_mass_fractions;
    void calcDensities(double& temp, double& pressure, double& density,
                       std::map<G4Element*,double>& element2density) const;
    G4Material * buildMat(const char * name, double temp, double pressure, double density,
                          const std::map<G4Element*,double>& element2density, double scale_e2d) const;
    std::string autoName() const;
  };
}
#endif
