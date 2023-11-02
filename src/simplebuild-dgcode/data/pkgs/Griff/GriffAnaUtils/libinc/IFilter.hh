#ifndef GriffAnaUtils_IFilter_hh
#define GriffAnaUtils_IFilter_hh

#include "Utils/RefCountBase.hh"

//Base class of the various filters, ensuring ref-counting and a common negation interface

namespace GriffAnaUtils {

  class IFilter : public Utils::RefCountBase {
  public:
    //NOTE: Destructor of derived classes must not be public
    //      for ref-counting to work properly!
    void setNegated(bool n=true) { m_negated = n; }
    void toggleNegation() { m_negated = !m_negated; }
    bool negated() const { return m_negated; }

    //Filter can be dynamically disabled, in which case it will pass everything:
    bool enabled() const { return m_enabled; }
    bool setEnabled(bool b = true) { bool old =m_enabled; m_enabled = b; return old; }
    bool setDisabled(bool b = true) { bool old = !m_enabled; m_enabled = !b; return old; }

  protected:
    IFilter() : m_negated(false), m_enabled(true) {}
    virtual ~IFilter(){}
  private:
    IFilter( const IFilter & );
    IFilter & operator= ( const IFilter & );
    bool m_negated;
    bool m_enabled;
  };
}

#endif
