#ifndef Mesh_Mesh_hh
#define Mesh_Mesh_hh

#include "Core/Types.hh"
#include "zlib.h"
#include "Mesh/MeshFiller.hh"
#include "Utils/DelayedAllocVector.hh"
#include "Utils/PackSparseVector.hh"
#include <functional>
#include <string>
#include <map>

namespace Mesh {
  template<unsigned NDIM>
  class Mesh {
  public:
    Mesh( const long(&ncells)[NDIM],
          const double(&cell_lower)[NDIM],
          const double(&cell_upper)[NDIM],
          const std::string& nname,
          const std::string& ccomments = "");
    ~Mesh(){}

    //Data:
    const std::string& name() const { return m_name; }
    const std::string& comments() const { return m_comments; }
    const std::string& cellunits() const { return m_cellunits; }
    typedef MeshFiller<NDIM,Utils::DelayedAllocVector<double> > TFiller;
    TFiller& filler() { return m_filler; }
    const TFiller& filler() const { return m_filler; }

    //Completely reinitialise:
    void reinit( const long(&ncells)[NDIM],
                 const double(&cell_lower)[NDIM],
                 const double(&cell_upper)[NDIM],
                 const std::string& nname,
                 const std::string& ccomments = "");

    //File-based serialisation:
    void saveToFile(const std::string& filename);
    Mesh(const std::string& filename);

    //Stream-based serialisation:
    typedef std::function<void(unsigned char* buf, unsigned buflen)> TDataAcceptor;
    typedef std::function<unsigned(unsigned char* buf, unsigned buflen)> TDataProvider;
    void write(TDataAcceptor&) const;
    void reinit(TDataProvider&);

    //Construct invalid object which must be completed by calling reinit(..):
    Mesh();
    bool isInvalid() const { return m_filler.isInvalid(); }

    //True if all metadata (cell layout, name, comments, not the data in
    //filler().data() is identical:
    bool compatible(Mesh& other);

    //Add contents from "other" to this instance (throws runtime error if not
    //compatible):
    void merge(Mesh& other);

    //Same, but directly from a stored file to avoid temporarily having both
    //objects in memory:
    void merge(const std::string& filename_other);

    void setName(const char* n) { m_name = n; };
    void setComments(const char* c) { m_comments = c; };
    void setCellUnits(const char* cu) { m_cellunits = cu; };

    //Optional scalar statistics can be attached to the Mesh. Initial value of a
    //statistics when enabled is 0.0, and values will be summed upon invocation
    //of Mesh::merge.

    double& enableStat( const std::string& );//add new stat and return ref to its value
    double& stat( const std::string& );
    const double& stat( const std::string& ) const;

    //Direct access to stat data:
    typedef std::map<std::string,double> TStatMap;
    const TStatMap& statMap() const { return m_stats; }

private:
    TFiller m_filler;
    TStatMap m_stats;
    std::string m_name;
    std::string m_comments;
    std::string m_cellunits;
    template <class T>
    void write(TDataAcceptor& da,const T& t) const;
    void extract(TDataProvider& dp,unsigned char * buf, unsigned buflen) const;
    template <class T>
    void extract(TDataProvider& dp,T& t) const;
    void extractstr(TDataProvider& dp,std::string& t) const;
    void extract_header(TDataProvider& dp, std::string& n, std::string& c, std::string& cu, TFiller&, TStatMap&);
    void extract_eof(TDataProvider& dp);
    void openFile(const std::string&, TDataProvider &, gzFile&);
    void merge_stats(Mesh& other);
  };

  ////////////////////////////
  // Inline implementations //
  ////////////////////////////

  template<unsigned NDIM>
  template <class T>
  inline void Mesh<NDIM>::write(TDataAcceptor& da,const T& t) const
  {
    da((unsigned char*)&t,sizeof(t));
  }

  template<unsigned NDIM>
  inline void Mesh<NDIM>::extract(TDataProvider& dp,unsigned char * buf, unsigned buflen) const
  {
    while (buflen>0) {
      unsigned n = dp(buf,buflen);
      if (n<1||n>buflen)
        throw std::runtime_error("Read error");
      buf += n;
      buflen -= n;
    }
  }

  template<unsigned NDIM>
  template <class T>
  inline void Mesh<NDIM>::extract(TDataProvider& dp,T& t) const
  {
    extract(dp,(unsigned char*)&t,(unsigned)sizeof(t));
  }

  template<unsigned NDIM>
  inline void Mesh<NDIM>::extractstr(TDataProvider& dp,std::string& t) const
  {
    t.clear();
    std::uint16_t sz;
    extract(dp,sz);
    if (sz) {
      t.resize(sz);
      extract(dp,(unsigned char*)t.c_str(),(unsigned)sz);
    }
  }

  template<unsigned NDIM>
  inline Mesh<NDIM>::Mesh( const long(&ncells)[NDIM],
                           const double(&cell_lower)[NDIM],
                           const double(&cell_upper)[NDIM],
                           const std::string& nname,
                           const std::string& ccomments)
    : m_filler(ncells,cell_lower,cell_upper),
      m_name(nname),
      m_comments(ccomments)
  {
  }

  template<unsigned NDIM>
  inline Mesh<NDIM>::Mesh()
  {
  }

  template<unsigned NDIM>
  inline
  void Mesh<NDIM>::openFile(const std::string& filename, TDataProvider & dp, gzFile& file)
  {
    assert(!dp&&!file);
    file = gzopen(filename.c_str(),"rb");
    if (!file)
      throw std::runtime_error("Unable to open input file!");

    dp = [&file](unsigned char* buf, unsigned buflen)
      {
        int nb = gzread(file, buf, buflen);
        if (nb<1||(unsigned)nb>buflen)
          throw std::runtime_error("Read error");
        return (unsigned)nb;
      };
  }

  template<unsigned NDIM>
  inline Mesh<NDIM>::Mesh(const std::string& filename)
  {
    TDataProvider dp;
    gzFile file = 0;
    openFile(filename,dp,file);
    reinit(dp);
    gzclose(file);
  }

  template<unsigned NDIM>
  inline void Mesh<NDIM>::reinit( const long(&ncells)[NDIM],
                                  const double(&cell_lower)[NDIM],
                                  const double(&cell_upper)[NDIM],
                                  const std::string& nname,
                                  const std::string& ccomments )
  {
    m_filler.reinit(ncells,cell_lower,cell_upper);
    m_name = nname;
    m_comments = ccomments;
    m_stats.clear();
  }

  template<unsigned NDIM>
  inline void Mesh<NDIM>::saveToFile(const std::string& filename)
  {
    gzFile file = gzopen(filename.c_str(),"wb9");//compression level 9
    if (!file)
      throw std::runtime_error("Unable to open output file!");
    TDataAcceptor da = [&file](unsigned char* buf, unsigned buflen)
      {
        while (buflen) {
          int nb = gzwrite(file,buf,buflen);
          if (nb<1||(unsigned)nb>buflen)
            throw std::runtime_error("Write error");
          buf += nb;
          buflen -= nb;
        }
      };
    write(da);
    gzclose(file);
  }

  template<unsigned NDIM>
  inline void Mesh<NDIM>::write(TDataAcceptor& da) const
  {
    //embed "MESH<NDIM>D" and format version:
    assert(NDIM>=1&&NDIM<=255-'0');
    unsigned char start[8] = {'M','E','S','H','0'+NDIM,'D','0','1'};
    write(da,start);

    //Name, comments, cell dimensions:
    if (m_name.size()>65000||m_comments.size()>65000)
      throw std::runtime_error("Write error - too large string");
    std::uint16_t tmp16 = m_name.size();
    write(da,tmp16);
    da((unsigned char*)m_name.c_str(),tmp16);
    tmp16 = m_comments.size();
    write(da,tmp16);
    da((unsigned char*)m_comments.c_str(),tmp16);
    tmp16 = m_cellunits.size();
    write(da,tmp16);
    da((unsigned char*)m_cellunits.c_str(),tmp16);
    for (unsigned i = 0; i<NDIM; ++i) {
      std::uint32_t tmp = m_filler.nCells(i);
      write(da,tmp);
      double cl = m_filler.cellLower(i);
      write(da,cl);
      double cu = m_filler.cellUpper(i);
      write(da,cu);
    }

    //Stats:
    tmp16 = m_stats.size();
    write(da,tmp16);
    for (auto it = m_stats.begin(); it!=m_stats.end(); ++it) {
      tmp16 = it->first.size();
      write(da,tmp16);
      da((unsigned char*)(it->first.c_str()),tmp16);
      write(da,it->second);
    }

    //Contents:
    Utils::PackSparseVector::write(m_filler.data(),da);

    //EOF marker:
    unsigned char end[7] = {'M','E','S','H','E','O','F'};
    write(da,end);
  }

  //static
  template<unsigned NDIM>
  inline void Mesh<NDIM>::extract_header(TDataProvider& dp,
                                         std::string& nname,
                                         std::string& ccomments,
                                         std::string& ccellunits,
                                         TFiller& ffiller,
                                         TStatMap& sstats)
  {
    //Embedded "MESH<NDIM>D" and format version:
    assert(NDIM<=255);
    unsigned char exp_start[8] = {'M','E','S','H','0'+NDIM,'D','0','1'}, start[8];
    extract(dp,start);
    for (unsigned i = 0; i<sizeof(start); ++i)
      if (exp_start[i]!=start[i])
        throw std::runtime_error("Read error - bad format");
    //Name, comments, cell dimensions:
    extractstr(dp,nname);
    extractstr(dp,ccomments);
    extractstr(dp,ccellunits);
    long ncells[NDIM];
    double cell_lower[NDIM];
    double cell_upper[NDIM];

    for (unsigned i = 0; i<NDIM; ++i) {
      std::uint32_t tmp;
      extract(dp,tmp);
      ncells[i] = tmp;
      extract(dp,cell_lower[i]);
      extract(dp,cell_upper[i]);
    }
    ffiller.reinit(ncells,cell_lower,cell_upper);

    //stats:
    sstats.clear();
    std::uint16_t nstats;
    std::string tmpstr;
    double tmpdbl;
    extract(dp,nstats);
    while (nstats>0) {
      --nstats;
      extractstr(dp,tmpstr);
      if (sstats.find(tmpstr)!=sstats.end())
        throw std::runtime_error("Read error - bad format");
      extract(dp,tmpdbl);
      sstats[tmpstr]=tmpdbl;
    }
  }

  template<unsigned NDIM>
  inline void Mesh<NDIM>::extract_eof(TDataProvider& dp)
  {
    //EOF marker:
    unsigned char exp_end[7] = {'M','E','S','H','E','O','F'}, end[7];
    extract(dp,end);
    for (unsigned i = 0; i<sizeof(end); ++i)
      if (exp_end[i]!=end[i])
        throw std::runtime_error("Read error - bad format");
  }

  template<unsigned NDIM>
  inline void Mesh<NDIM>::reinit(TDataProvider& dp)
  {
    extract_header(dp, m_name, m_comments, m_cellunits, m_filler, m_stats);
    Utils::PackSparseVector::read(m_filler.data(),dp);
    extract_eof(dp);
  }

  template<unsigned NDIM>
  inline double & Mesh<NDIM>::enableStat( const std::string&ss )
  {
    if (m_stats.find(ss)==m_stats.end()) {
      if (m_stats.size()>10000)
        throw std::runtime_error("Too many stats enabled");
      if (ss.size()>10000)
        throw std::runtime_error("Too long stat name");
      m_stats[ss] = 0.0;
    }
    return m_stats[ss];
  }

  template<unsigned NDIM>
  inline double& Mesh<NDIM>::stat( const std::string& ss )
  {
    auto it = m_stats.find(ss);
    if (it==m_stats.end())
      throw std::runtime_error("Trying to access unavailable statistics");
    return it->second;
  }

  template<unsigned NDIM>
  inline const double& Mesh<NDIM>::stat( const std::string& ss ) const
  {
    auto it = m_stats.find(ss);
    if (it==m_stats.end())
      throw std::runtime_error("Trying to access unavailable statistics");
    return it->second;
  }

  template<unsigned NDIM>
  inline bool Mesh<NDIM>::compatible(Mesh& other)
  {
    if (m_name != other.m_name
        || m_comments != other.m_comments
        || m_cellunits != other.m_cellunits
        || m_stats.size() != other.m_stats.size()
        || !m_filler.compatible(other.m_filler) )
      return false;
    auto it = m_stats.begin();
    auto itE = m_stats.end();
    auto itO = other.m_stats.begin();
    for (;it!=itE;++it,++itO)
      if (it->first!=itO->first)
        return false;
    return true;
  }

  template<unsigned NDIM>
  inline void Mesh<NDIM>::merge_stats(Mesh& other)
  {
    auto it = m_stats.begin();
    auto itE = m_stats.end();
    auto itO = other.m_stats.begin();
    for (;it!=itE;++it,++itO) {
      assert(itO!=other.m_stats.end());
      assert(it->first==itO->first);
      it->second += itO->second;
    }
    assert(itO==other.m_stats.end());
  }

  //Add contents from compatible "other" to this instance:
  template<unsigned NDIM>
  inline void Mesh<NDIM>::merge(Mesh& other)
  {
    if (!compatible(other))
      throw std::runtime_error("Trying to merge contents of incompatible object");
    merge_stats(other);
    filler().data().merge(other.filler().data());
  }

  template<unsigned NDIM>
  inline void Mesh<NDIM>::merge(const std::string& filename_other)
  {
    TDataProvider dp;
    gzFile file = 0;
    openFile(filename_other,dp,file);
    Mesh other;
    //Just extract the header of the other object and use it to verify
    //compatibility and to access it's stat values:
    extract_header(dp, other.m_name, other.m_comments, other.m_cellunits, other.m_filler, other.m_stats);
    if (!compatible(other)) {
      gzclose(file);
      throw std::runtime_error("Trying to merge contents of incompatible object");
    }
    merge_stats(other);

    //The data goes directly into our data storage:
    Utils::PackSparseVector::read(m_filler.data(),dp);

    //Check valid EOF and close:
    extract_eof(dp);
    gzclose(file);
  }
}

#endif
