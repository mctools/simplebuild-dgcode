#ifndef IdealGasBuilder_IdealGasComponent_hh
#define IdealGasBuilder_IdealGasComponent_hh

//Component class. See IdealGasBuilder.hh for a description of the purpose of
//this class, as well as the syntax for the "formula" arguments below.
//
//Author: Thomas Kittelmann, January 2015.

#include <string>
#include <vector>
#include <utility>
class G4Element;
class G4Material;

namespace IdealGas {

  class Component {
  public:

    ////////////////////////////////////////////////////////////////////////
    //                           Set up component                         //
    ////////////////////////////////////////////////////////////////////////

    //Use this constructor to quickly create a locked component using a
    //component formula (see IdealGasBuilder.hh section 2.2):
    Component(const char * formula);

    //Empty constructor. When using this, you must subsequently define the
    //component with addElement/addElements calls:
    Component();

    //Add an element and an associated relative count. Note that the class will
    //keep a pointer to the element, but will not assume ownership of it:
    void addElement(G4Element* element, double count);

    //Convenience version taking an element formula (see IdealGasBuilder.hh section 2.1):
    void addElement(const char* formula, double count);

    //Add multiple elements at once using a component formula (see
    //IdealGasBuilder.hh section 2.2):
    void addElements(const char * formula);

    //lock instance, causing further calls to addElement/addElements to throw errors:
    void lock() const;

    ////////////////////////////////////////////////////////////////////////
    //  Access information about the component (implies lock())           //
    ////////////////////////////////////////////////////////////////////////

    //Mass of a mole  of the substance:
    double molarMass() const;

    //Component name, automatically generated from element names and their counts:
    const char * name() const;

    //Access info about the contained elements:
    unsigned nElements() const;
    G4Element* element(unsigned ielement) const;
    double molarMassByElement(unsigned ielement) const;

    ////////////////////////////////////////////////////////////////////////
    //  Use component to generate G4 materials (implies lock())           //
    ////////////////////////////////////////////////////////////////////////

    // Create G4 material representing the gas (see IdealGasBuilder.hh section 3.0):
    G4Material * createMaterialCalcD(double temp = -1, double pressure = -1, const char * name = 0);//Density will be calculated
    G4Material * createMaterialCalcP(double density, double temp = -1, const char * name = 0);//Pressure will be calculated
    G4Material * createMaterialCalcT(double density, double pressure = -1, const char * name = 0);//Temperature will be calculated
    G4Material * createMaterial(double temp = -1, double pressure = -1, const char * name = 0);//Same as createMaterialCalcD

    //Destructor (doesn't do anything):
    ~Component();

  private:
    std::vector<std::pair<G4Element*,double> > m_elements;
    std::string m_name;
    mutable double m_mass;
    mutable bool m_unlocked;
  };

  //This function, which might in principle be useful outside the context of
  //ideal gasses, creates a G4Element instance from an element formula (see
  //IdealGasBuilder.hh section 2.1):
  G4Element * getElementFromFormula(const char* formula);

}

#endif
