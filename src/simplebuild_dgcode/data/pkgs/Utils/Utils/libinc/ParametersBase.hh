#ifndef Utils_ParametersBase_hh
#define Utils_ParametersBase_hh

#include <string>
#include <vector>
#include <set>

namespace Utils {

  //Base class for objects which must be configurable with a declarable and
  //printable list of parameters. Each parameter must be either a bool, a
  //double, an integer or a string. In the case of ints and doubles, it is
  //possible to define an allowed range of values. For any parameter, one can
  //override validateParameters and implement more complex parameter validation
  //(Examples: the value of one parameter influences the allowed range of another
  //parameter or a string parameter for which only certain values are allowed).

  class ParametersBase {
  public:
    typedef std::vector<std::string> ParameterList;
    typedef std::set<std::string> ParameterSet;

    //Forbid copy/move completely, which seems safest:
    ParametersBase( const ParametersBase& ) = delete;
    ParametersBase& operator=( const ParametersBase& ) = delete;
    ParametersBase( ParametersBase&& ) = delete;
    ParametersBase& operator=( ParametersBase&& ) = delete;

    ParametersBase();
    virtual ~ParametersBase();

    //Set parameters by the following methods
    void setParameterDouble(const std::string&name, double value);
    void setParameterInt(const std::string&name, int value);
    void setParameterBoolean(const std::string&name, bool value);
    void setParameterString(const std::string&name, const std::string& value);
    void lock();//call this to prevent further changes to parameters. Called automatically upon first call to any getParameterXXX(..)
    bool isLocked() const;

    //To access parameter settings (note, this locks the instance!):
    double getParameterDouble(const std::string&name) const;
    int getParameterInt(const std::string&name) const;
    double getParameterBoolean(const std::string&name) const;
    const std::string& getParameterString(const std::string&name) const;
    const char * getParameterStringCStr(const std::string&name) const { return getParameterString(name).c_str(); }

    void getParameterString_SplitAsVector(const std::string&name, std::vector<std::string>&) const;

    //Print parameter settings to stdout:
    virtual void dump(const char* prefix="") const;
    virtual void dumpNoLock(const char* prefix="") const;

    //Query available methods:
    bool hasParameterDouble(const std::string& name) const;
    bool hasParameterInt(const std::string& name) const;
    bool hasParameterBoolean(const std::string& name) const;
    bool hasParameterString(const std::string& name) const;
    void getParameterListDouble(ParameterList&) const;
    void getParameterListInt(ParameterList&) const;
    void getParameterListBoolean(ParameterList&) const;
    void getParameterListString(ParameterList&) const;

    //Serialised representation of parameters and current values. Can be
    //used to instantiate DummyParamHolder objects with similar parameters and
    //values as the original object.
    void serialiseParameters(char*& output, unsigned& outputLength) const;
    void noHardExitOnParameterFailure();//call to trigger runtime exceptions rather than exit(1) calls in case of problems.

    //Tie two parameters, so changes on one is propagated to the other (the
    //"other" value immediately becomes the initial common value). All parameter
    //constraints will have to be fulfilled as usual:
    void tieParameters(const std::string&name, ParametersBase * other, const std::string& other_name);

    //Convenience overload to tie two parameters on the present object. The
    //current value of the name2 parameter will become the new common value.
    void tieParameters(const std::string&name1, const std::string& name2);

    //Expose a parameter from "other", possibly with a different name (if
    //new_name is provided). This means that we add a new corresponding
    //parameter by addParameterXXX, and subsequently tie it to the one on
    //"other" with tieParameters(..). Set tieOnClash to tie the parameters if
    //they clash with existing parameters:
    void exposeParameter(const std::string& other_name, ParametersBase * other, const std::string& new_name = "", bool tieOnClash = false);

    //Expose *all* parameters from "other", possibly with a prefix on the new
    //names.
    void exposeParameters(ParametersBase * other, const std::string& prefix = "", bool tieOnClash = false );

    //Expose all paramters, except those in an exclusion list:
    void exposeParameters(ParametersBase * other, const ParameterSet& excluded_pars, const std::string& prefix = "", bool tieOnClash = false );

    //Ideas for future uses of the "tie" feature:
    //  tieAllParameters(ParametersBase * other, const std::string& prefix = "");
    //  tieParameters(ParametersBase * other, const ParameterList&, const std::string& prefix = "");
  protected:
    //To be called in the constructor of derived classes, defining which parameters are available:
    void addParameterDouble(const std::string&name, double default_value);
    void addParameterDouble(const std::string&name, double default_value, double valmin, double valmax);
    void addParameterInt(const std::string&name, int default_value);
    void addParameterInt(const std::string&name, int default_value, int valmin, int valmax);
    void addParameterBoolean(const std::string&name, bool default_value);
    void addParameterString(const std::string&name, const std::string& default_value);

    virtual bool validateParameters() { return true; }//called when settings are locked, to ensure consistency beyond the min/max checks.
    //NB: it is still allowed to set parameters inside
    //validateParameters(). This is to handle cases where one parameter has to
    //be changed due to the contents of another parameter.

    //TODO, allowed-chars and length min/max:
    // void addParameterString(const std::string&name, const std::string& default_value, const std::string& allowedChars);
    // void addParameterString(const std::string&name, const std::string& default_value, unsigned length_min,unsigned length_max);
    // void addParameterString(const std::string&name, const std::string& default_value,
    //                         const std::string& allowedChars,unsigned length_min,unsigned length_max);

  public:
    //Access without locking. This is for the python interface, and is not
    //intended to be used from C++ unless you know what you are doing and
    //understand the reason for locking in the first place:
    double getParameterDoubleNoLock(const std::string&name) const;
    int getParameterIntNoLock(const std::string&name) const;
    bool getParameterBooleanNoLock(const std::string&name) const;
    const std::string& getParameterStringNoLock(const std::string&name) const;
    const char * getParameterStringCStrNoLock(const std::string&name) const { return getParameterStringNoLock(name).c_str(); }
    void setIgnoreRanges();//disable the ranges checks (use sparingly)

    void setParameter(const std::string&name, double value) { setParameterDouble(name,value); }
    void setParameter(const std::string&name, int value) { setParameterInt(name,value); }
    void setParameter(const std::string&name, bool value) { setParameterBoolean(name,value); }
    void setParameter(const std::string&name, const std::string& value) { setParameterString(name,value); }

  private:
    struct Imp;
    Imp * m_imp;
  };
}

#endif
