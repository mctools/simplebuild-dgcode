#ifndef G4OSG_ObjectHolder_hh
#define G4OSG_ObjectHolder_hh

#include <osg/Referenced>

namespace G4OSG {

  //To capture Handles (VolHandle, TrkHandle) in osg objects as user data.

  class ObjectHolder : public osg::Referenced {
  public:
    ObjectHolder(void*h, int t) : m_h(h), m_type(t) {}
    ~ObjectHolder(){}
    int type() const { return m_type; }
    template<class T>
    T * handle() const { return (T*)m_h; }
  private:
    void * m_h;
    int m_type;
  };

}

#endif
