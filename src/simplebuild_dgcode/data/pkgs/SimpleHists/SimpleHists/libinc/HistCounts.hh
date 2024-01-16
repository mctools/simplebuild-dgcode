#ifndef SimpleHists_HistCounts_hh
#define SimpleHists_HistCounts_hh

//A special histogram for keeping track of a series of labelled counters.
//
//It is intended that one adds a number of named counters right after
//construction, and then proceed to increment them during processing, using the
//Counter object returned from addCounter(..).
//But of course, counters can also be added later, on demand.

#include "SimpleHists/HistBase.hh"
#include <cmath>
#include <list>
#include <map>

namespace SimpleHists {

  class HistCounts : public HistBase {
  public:

    HistCounts();
    HistCounts(const std::string& title_or_serialised_data);//the class is smart enough to know if it
                                                            //is a title or a serialised histogram
    virtual ~HistCounts();

    class Counter;//Pointer sized object which can be passed around by value (see declaration below)

    Counter addCounter(const std::string& label, const std::string& display_text = "");
    const Counter getCounter(const std::string& label) const;
    Counter getCounter(const std::string& label);

    bool hasCounter(const std::string& label) const;

    virtual unsigned dimension() { return 1; }
    virtual void dump(bool contents = false, const std::string& prefix = "") const;

    double getMaxContent() const;

    virtual bool empty() const;
    size_t nCounters() const;
    virtual double getIntegral() const;

    virtual char histType() const { return 0x03; }
    virtual void serialise(std::string&) const;

    //Merge contents of another compatible histogram onto this one.
    virtual bool mergeCompatible(const HistBase*) const;//check this before calling next method
    virtual void merge(const HistBase*);

    void setErrorsByContent();//After this errors will be sqrt(content)

    virtual bool isSimilar(const HistBase*) const;

    virtual void scale(double scalefact);

    virtual HistBase* clone() const;

    virtual void reset();//all counters will be reset

    //resort added counters (default is order of addCounter calls):
    void sortByLabels();
    void sortByDisplayLabels();

    void getCounters(std::list<Counter>&);//all counters will be appended in order

  private:
    void perform_deserialisation(const std::string& serialised_data);

    //Copy/assignment is forbidden:
    HistCounts( const HistCounts & );
    HistCounts & operator= ( const HistCounts & );

    struct CounterInternal;
    std::list<CounterInternal> m_counters;
    std::map<std::string,CounterInternal*> m_label2counter;

  };

  class HistCounts::Counter {
  public:
    double getValue() const;
    double getError() const;
    double getErrorSquared() const;
    const std::string& getLabel() const;
    const std::string& getDisplayLabel() const;
    const std::string& getComment() const;
    void setDisplayLabel(const std::string&);
    void setComment(const std::string&);
    Counter& operator++();
    Counter& operator+=(const double&);
    Counter& operator+=(const Counter&);
    Counter() : m_counter(0) {}
  private:
    friend class HistCounts;
    Counter(CounterInternal *);
    CounterInternal * m_counter;
  };

}

#include "SimpleHists/HistCounts.icc"

#endif
