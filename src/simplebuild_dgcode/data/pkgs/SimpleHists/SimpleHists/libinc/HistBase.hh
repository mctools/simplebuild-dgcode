#ifndef SimpleHists_HistBase_hh
#define SimpleHists_HistBase_hh

#include "Core/Types.hh"
#include <string>
#include <cassert>

//Base class for all simple histograms

namespace SimpleHists {

  class HistBase {
  public:
    HistBase();
    HistBase(const std::string& title);
    virtual ~HistBase();

    //Annotations (for plots):
    const std::string& getTitle() const;
    const std::string& getXLabel() const;
    const std::string& getYLabel() const;
    void setTitle(const std::string& t);
    void setXLabel(const std::string& l);
    void setYLabel(const std::string& l);

    //Comment (not displayed in plots, only in dumps/tooltips)
    const std::string& getComment() const;
    void setComment(const std::string& l);

    virtual bool empty() const = 0 ;//true after first fill with non-zero weight
    virtual double getIntegral() const = 0;
    virtual unsigned dimension() const = 0;//return histogram dimension (1 or 2)
    virtual void dump(bool contents = false, const std::string& prefix = "") const = 0;

    //methods for serialisation:
    virtual char histType() const = 0;//return histogram type (0x01 => Hist1D, 0x02 => Hist2D, 0x03 => HistCounts)
    virtual void serialise(std::string&) const = 0;//passed string will be overridden with serialised data

    //Merge contents of another compatible histogram into this one.
    virtual bool mergeCompatible(const HistBase*) const;//check for identical type, annotations and bin range
    virtual void merge(const HistBase*) = 0;

    //Test (up to some float tolerance) that two histograms have essentially identical metadata and contents.
    virtual bool isSimilar(const HistBase*) const;

    //Rescale weights (as if all fill calls had been done with weights w*scalefact rather than w)
    virtual void scale(double scalefact) = 0;

    //normalise (if non-empty histogram this is similar to scale(1/getIntegral()))
    void norm();

    //Create identical copy of the histogram:
    virtual HistBase* clone() const = 0;

    //Reset to unfilled state:
    virtual void reset() = 0;

  private:
    std::string m_title;
    std::string m_xLabel;
    std::string m_yLabel;
    std::string m_comment;

    //Copy/assignment is forbidden:
    HistBase( const HistBase & );
    HistBase & operator= ( const HistBase & );

  protected:
    void dumpBase(const char *) const;
    unsigned serialisedBaseSize() const;
    void serialiseBaseToBuffer(char*) const;
    void deserialiseBase(const std::string&,unsigned& offset,char& version);//offset points at first byte of the data for the derived
  };

  //A few global functions, which can be used to deal with serialised histogram
  //data:

  //return histogram type of serialised data (0 indicates error, but might not catch all problems):
  char histTypeOfData(const std::string& serialised_data);

  //creates and returns histogram of appropriate type
  HistBase * deserialise(const std::string& serialised_data);

}

#include "SimpleHists/HistBase.icc"

#endif
