#include "G4Interfaces/GeoBase.hh"
#include "G4Materials/NamedMaterialProvider.hh"
#include "G4RotationMatrix.hh"
#include "G4VSolid.hh"
#include "G4VisAttributes.hh"
#include <map>
#include <iostream>

G4Interfaces::GeoBase::GeoBase(const char* name)
  : m_name(name),
    m_checkOverlaps(true)
{
}

G4Interfaces::GeoBase::~GeoBase()
{
}

void G4Interfaces::GeoBase::dump(const char * prefix) const {
  std::cout<<std::flush;
  std::cout<<prefix<<"GeoConstructor["<<m_name<<"]:"<<std::endl;
  std::string p = prefix;
  p+="  ";
  Utils::ParametersBase::dump(p.c_str());
}

class impGeoBase__compare_colour {
public:
  bool operator()(const G4Colour c1, const G4Colour c2) const
  {
    if (c1.GetRed()!=c2.GetRed()) return c1.GetRed()<c2.GetRed();
    if (c1.GetGreen()!=c2.GetGreen()) return c1.GetGreen()<c2.GetGreen();
    if (c1.GetBlue()!=c2.GetBlue()) return c1.GetBlue()<c2.GetBlue();
    if (c1.GetAlpha()!=c2.GetAlpha()) return c1.GetAlpha()<c2.GetAlpha();
    return false;//equals
  }
};
const G4Colour G4Interfaces::GeoBase::INVISIBLE  = G4Colour(1,   1,   1,   0);
const G4Colour G4Interfaces::GeoBase::WHITE      = G4Colour(1,   1,   1,   1);
const G4Colour G4Interfaces::GeoBase::SILVER     = G4Colour(0.75,0.75,0.75,1);
const G4Colour G4Interfaces::GeoBase::GOLD       = G4Colour(0.83,0.69,0.21,1);
const G4Colour G4Interfaces::GeoBase::GRAY       = G4Colour(0.5, 0.5, 0.5, 1);
const G4Colour G4Interfaces::GeoBase::RED        = G4Colour(1,   0,   0,   1);
const G4Colour G4Interfaces::GeoBase::DARKRED    = G4Colour(0.5, 0,   0,   1);
const G4Colour G4Interfaces::GeoBase::GREEN      = G4Colour(0,   1,   0,   1);
const G4Colour G4Interfaces::GeoBase::DARKGREEN  = G4Colour(0,   0.5, 0,   1);
const G4Colour G4Interfaces::GeoBase::BLUE       = G4Colour(0,   0,   1,   1);
const G4Colour G4Interfaces::GeoBase::DARKBLUE   = G4Colour(0,   0,   0.5, 1);
const G4Colour G4Interfaces::GeoBase::YELLOW     = G4Colour(1,   1,   0,   1);
const G4Colour G4Interfaces::GeoBase::DARKYELLOW = G4Colour(0.5, 0.5, 0,   1);
const G4Colour G4Interfaces::GeoBase::ORANGE     = G4Colour(1,   0.65,0,   1);
const G4Colour G4Interfaces::GeoBase::DARKORANGE = G4Colour(0.78,0.32,0.01,1);
const G4Colour G4Interfaces::GeoBase::BLACK      = G4Colour(0,   0,   0,   1);
const G4Colour G4Interfaces::GeoBase::PURPLE     = G4Colour(1,   0,   1,   1);
const G4Colour G4Interfaces::GeoBase::DARKPURPLE = G4Colour(0.5, 0,   0.5, 1);
const G4Colour G4Interfaces::GeoBase::MAROON     = G4Interfaces::GeoBase::DARKRED;
const G4Colour G4Interfaces::GeoBase::OLIVE      = G4Interfaces::GeoBase::DARKYELLOW;
const G4Colour G4Interfaces::GeoBase::LIME       = G4Interfaces::GeoBase::GREEN;
const G4Colour G4Interfaces::GeoBase::CYAN       = G4Colour(0,   1,   1,   1);
const G4Colour G4Interfaces::GeoBase::DARKCYAN   = G4Colour(0,   0.5, 0.5, 1);
const G4Colour G4Interfaces::GeoBase::AQUA       = G4Interfaces::GeoBase::CYAN;
const G4Colour G4Interfaces::GeoBase::TEAL       = G4Interfaces::GeoBase::DARKCYAN;
const G4Colour G4Interfaces::GeoBase::NAVY       = G4Interfaces::GeoBase::DARKBLUE;
const G4Colour G4Interfaces::GeoBase::FUCHSIA    = G4Interfaces::GeoBase::PURPLE;
const G4Colour G4Interfaces::GeoBase::MAGENTA    = G4Interfaces::GeoBase::PURPLE;
const G4Colour G4Interfaces::GeoBase::BROWN      = G4Colour(0.63,0.32,0.18,1);
const G4Colour G4Interfaces::GeoBase::DARKBROWN  = G4Colour(0.55,0.27,0.07,1);

G4Interfaces::GeoBase::PlacedVols G4Interfaces::GeoBase::place(G4VSolid* shape,G4Material*mat,double x,double y,double z,
                                                                                 G4LogicalVolume*mother,const G4Colour&col,
                                                                                 G4int copyNum, const char * name,
                                                                                 G4RotationMatrix* rot)
{
  //TODO: Automatically cache and reuse logical volume when shape, mat and name are unchanged!!
  auto lv = new G4LogicalVolume(shape, mat, name ? G4String(name) : shape->GetName());
  auto pv = new G4PVPlacement(rot, G4ThreeVector(x,y,z), lv, name ? G4String(name) : shape->GetName(), mother, false, copyNum, m_checkOverlaps);
  if (col.GetAlpha()<1.0e-13) {
    lv->SetVisAttributes(G4VisAttributes::GetInvisible());
  } else {
    static std::map<G4Colour,G4VisAttributes*,impGeoBase__compare_colour> cache;
    auto it = cache.find(col);
    if (it!=cache.end()) {
      lv->SetVisAttributes(it->second);
    } else {
      auto vis = new G4VisAttributes(col);
      cache[col]=vis;
      lv->SetVisAttributes(vis);
    }
  }
  PlacedVols p;
  p.logvol = lv;
  p.physvol = pv;
  return p;
}

G4Interfaces::GeoBase::PlacedVols G4Interfaces::GeoBase::place(G4VSolid* shape,G4Material*mat,const G4Transform3D&trf,
                                                                                 G4LogicalVolume*mother,const G4Colour&col,
                                                                                 G4int copyNum, const char * name)
{
  auto lv = new G4LogicalVolume(shape, mat, name ? G4String(name) : shape->GetName());
  auto pv = new G4PVPlacement(trf, lv, name ? G4String(name) : shape->GetName(), mother, false, copyNum, m_checkOverlaps);
  if (col.GetAlpha()<1.0e-13) {
    lv->SetVisAttributes(G4VisAttributes::GetInvisible());
  } else {
    static std::map<G4Colour,G4VisAttributes*,impGeoBase__compare_colour> cache;
    auto it = cache.find(col);
    if (it!=cache.end()) {
      lv->SetVisAttributes(it->second);
    } else {
      auto vis = new G4VisAttributes(col);
      cache[col]=vis;
      lv->SetVisAttributes(vis);
    }
  }
  PlacedVols p;
  p.logvol = lv;
  p.physvol = pv;
  return p;
}

G4Interfaces::GeoBase::PlacedVols G4Interfaces::GeoBase::place(G4LogicalVolume* lv,double x,double y,double z,
                                                     G4LogicalVolume*mother,const G4Colour&col,
                                                     G4int copyNum, const char * name,
                                                     G4RotationMatrix* rot)
{
  auto pv = new G4PVPlacement(rot, G4ThreeVector(x,y,z), lv, name ? G4String(name) : lv->GetName(), mother, false, copyNum, m_checkOverlaps);
  if (col.GetAlpha()<1.0e-13) {
    lv->SetVisAttributes(G4VisAttributes::GetInvisible());
  } else {
    static std::map<G4Colour,G4VisAttributes*,impGeoBase__compare_colour> cache;
    auto it = cache.find(col);
    if (it!=cache.end()) {
      lv->SetVisAttributes(it->second);
    } else {
      auto vis = new G4VisAttributes(col);
      cache[col]=vis;
      lv->SetVisAttributes(vis);
    }
  }
  PlacedVols p;
  p.logvol = lv;
  p.physvol = pv;
  return p;
}

G4Interfaces::GeoBase::PlacedVols G4Interfaces::GeoBase::place(G4LogicalVolume* lv,const G4Transform3D&trf,
                                                                                 G4LogicalVolume*mother,const G4Colour&col,
                                                                                 G4int copyNum, const char * name)
{
  auto pv = new G4PVPlacement(trf, lv, name ? G4String(name) : lv->GetName(), mother, false, copyNum, m_checkOverlaps);
  if (col.GetAlpha()<1.0e-13) {
    lv->SetVisAttributes(G4VisAttributes::GetInvisible());
  } else {
    static std::map<G4Colour,G4VisAttributes*,impGeoBase__compare_colour> cache;
    auto it = cache.find(col);
    if (it!=cache.end()) {
      lv->SetVisAttributes(it->second);
    } else {
      auto vis = new G4VisAttributes(col);
      cache[col]=vis;
      lv->SetVisAttributes(vis);
    }
  }
  PlacedVols p;
  p.logvol = lv;
  p.physvol = pv;
  return p;
}

G4Material * G4Interfaces::GeoBase::getParameterMaterial(const std::string& s) const
{
  return NamedMaterialProvider::getMaterial(getParameterString(s));
}

G4Material * G4Interfaces::GeoBase::getMaterial(const std::string& s) const
{
  return NamedMaterialProvider::getMaterial(s);
}
