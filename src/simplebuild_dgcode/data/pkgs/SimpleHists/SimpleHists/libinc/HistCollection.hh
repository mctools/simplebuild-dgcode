#ifndef SimpleHists_HistCollection_hh
#define SimpleHists_HistCollection_hh

#include "SimpleHists/Hist1D.hh"//for convenience
#include "SimpleHists/Hist2D.hh"//for convenience
#include "SimpleHists/HistCounts.hh"//for convenience
#include <map>
#include <set>

namespace SimpleHists {

  struct AutoSave_t {};
  constexpr AutoSave_t AutoSave = AutoSave_t{};

  class HistCollection {
  public:
    //Initialise new (empty) collection (normally you would proceed to book histograms):
    HistCollection();

    //HistCollection which will automatically call saveToFile(filename,true)
    //from it's destructor:
    HistCollection( AutoSave_t, const std::string& filename );

    //Load histogram collection which was previously persistified by saveToFile(..):
    HistCollection(const std::string& filename);

    virtual ~HistCollection();//deallocates all contained histogram instances

    //Book histograms to be managed by the collection and use the returned
    //pointers to fill the histograms before displaying or persistifiying.
    //
    //Note that in addition to usual histogram parameters it is also necessary
    //to specify a unique key which can be used to retrieve the histogram.
    //
    //Allowed keys must have a length 1..60, at least one alphabetic character
    //("a-zA-Z"), and all characters must be either alphanumeric ("a-zA-Z0-9")
    //or one of "-./+_%=:". They can not end with ".shist".

    Hist1D* book1D(unsigned nbins, double xmin, double xmax, const std::string& key);
    Hist1D* book1D(const std::string& title,unsigned nbins, double xmin, double xmax, const std::string& key);
    Hist2D* book2D(unsigned nbinsx, double xmin, double xmax,
                   unsigned nbinsy, double ymin, double ymax, const std::string& key);
    Hist2D* book2D(const std::string& title,
                   unsigned nbinsx, double xmin, double xmax,
                   unsigned nbinsy, double ymin, double ymax, const std::string& key);
    HistCounts* bookCounts(const std::string& key);
    HistCounts* bookCounts(const std::string& title, const std::string& key);

    //Instead of booking, one can add existing hists (the HistCollection will own them afterwards)
    void add(HistBase*,const std::string&key);

    //Remove histogram from the collection (caller owns the hist afterwards)
    HistBase* remove(const std::string&key);

    //Expensive search for key associated to histogram (returns empty string if not part of this collection)
    const std::string& getKey(const HistBase*) const;

    //test if key exists:
    bool hasKey(const std::string& key) const;

    //retrieve histogram by key:
    HistBase* hist(const std::string& key);
    const HistBase* hist(const std::string& key) const;

    //persistification:
    virtual void saveToFile(const std::string& filename, bool allowOverwrite = false) const;

    //Browse and access (for interactive tools - not necessarily super efficient):
    void getKeys(std::set<std::string>& keys) const;//keys will be inserted to passed list

    const std::map<std::string,HistBase*>& getHistograms() const { return m_hists; }

    //Merge contents of other compatible histograms onto this one.
    void merge(const HistCollection*);

    //Merge directly from other file (NB: For now the other file will be completely loaded in memory!)
    void merge(const std::string& filename_other_collection);

    //Must have (up to floating point precision) similar histograms, including contents and errors.
    bool isSimilar(const HistCollection*) const;

    //Delete contained histograms (called automatically in destructor):
    void clear();

    //Only allow move construction, no copy/assign:
    HistCollection( HistCollection && o ) { m_hists.clear(); std::swap(m_hists,o.m_hists); }
    HistCollection & operator= ( HistCollection && rh ) { clear(); m_hists.clear(); std::swap(m_hists,rh.m_hists); return *this; }

    //Add clones of all histograms found in the other collection. Ignores any
    //histogram with a key already present in this instance:
    void cloneMissing( const HistCollection& o );

  private:
    //Contained histograms, in order of keys
    std::map<std::string,HistBase*> m_hists;
    std::string m_autosavefilename;

    void testKey(const std::string& key);

    //Forbid copy/assignment:
    HistCollection( const HistCollection & ) = delete;
    HistCollection & operator= ( const HistCollection & ) = delete;
  };

}

#endif
