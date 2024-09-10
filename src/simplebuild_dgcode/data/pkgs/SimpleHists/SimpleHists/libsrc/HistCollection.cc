#include "Core/String.hh"
#include "zlib.h"//gzopen, ...
#include "SimpleHists/HistCollection.hh"
#include <cassert>
#include <stdexcept>
#include "Core/File.hh"
#include <cstring>
#include <iostream>

#define MAGICWORD 0x51415709//for .shist files

SimpleHists::HistCollection::HistCollection()
{
}

SimpleHists::HistCollection::HistCollection( AutoSave_t, const std::string& filename )
  : m_autosavefilename( filename )
{
}

SimpleHists::HistCollection::~HistCollection()
{
  if ( !m_autosavefilename.empty() ) {
    saveToFile(m_autosavefilename,true); // cppcheck-suppress throwInNoexceptFunction
    std::cout<<"Saved SimpleHists collection to "<<m_autosavefilename<<std::endl;
  }
  clear();
}

void SimpleHists::HistCollection::clear()
{
  auto itE = m_hists.end();
  for (auto it = m_hists.begin();it!=itE;++it)
    delete it->second;
  m_hists.clear();
}

SimpleHists::Hist1D* SimpleHists::HistCollection::book1D(unsigned nbins, double xmin, double xmax,
                                                         const std::string& key)
{
  testKey(key);
  Hist1D * h = new Hist1D(nbins,xmin,xmax);
  m_hists[key]=h;
  return h;
}

void SimpleHists::HistCollection::cloneMissing( const HistCollection& o )
{
  for( auto& e : o.m_hists ) {
    if ( !m_hists.count( e.first ) )
      m_hists[ e.first ] = e.second->clone();
  }
}

SimpleHists::Hist1D* SimpleHists::HistCollection::book1D(const std::string& title,
                                                         unsigned nbins, double xmin, double xmax,
                                                         const std::string& key)
{
  testKey(key);
  Hist1D * h = new Hist1D(title,nbins,xmin,xmax);
  m_hists[key]=h;
  return h;
}

SimpleHists::Hist2D* SimpleHists::HistCollection::book2D(unsigned nbinsx, double xmin, double xmax,
                                                         unsigned nbinsy, double ymin, double ymax,
                                                         const std::string& key)
{
  testKey(key);
  Hist2D * h = new Hist2D(nbinsx,xmin,xmax,nbinsy,ymin,ymax);
  m_hists[key]=h;
  return h;
}

SimpleHists::Hist2D* SimpleHists::HistCollection::book2D(const std::string& title,
                                                         unsigned nbinsx, double xmin, double xmax,
                                                         unsigned nbinsy, double ymin, double ymax,
                                                         const std::string& key)
{
  testKey(key);
  Hist2D * h = new Hist2D(title,nbinsx,xmin,xmax,nbinsy,ymin,ymax);
  m_hists[key]=h;
  return h;
}

SimpleHists::HistCounts* SimpleHists::HistCollection::bookCounts(const std::string& key)
{
  testKey(key);
  HistCounts * h = new HistCounts();
  m_hists[key]=h;
  return h;
}

SimpleHists::HistCounts* SimpleHists::HistCollection::bookCounts(const std::string& title,const std::string& key)
{
  testKey(key);
  HistCounts * h = new HistCounts(title);
  m_hists[key]=h;
  return h;
}

void SimpleHists::HistCollection::testKey(const std::string& key)
{
  //Make useful as python attributes? I.e. alphanum+'_' only, and must start with alphanum (unless we add alphanum?)

  //Correct keys have a length 1..60, at least one alphabetic character
  //("a-zA-Z"), and all characters must be either alphanumeric ("a-zA-Z0-9") or
  //one of "-./+_%=:". They can not end with ".shist".
  //
  //The reason for these restrictions are that cmdline interfaces, guis, and
  //interactive python sessions dealing with shist files can be made more
  //intuitive and powerful.

  if (key.empty()) {
    printf("HistCollection ERROR empty key\n");
    throw std::runtime_error("HistCollection key is empty");
  }
  if (key.size()>60) {
    printf("HistCollection ERROR key too long (max allowed 60 chars): \"%s\"\n",key.c_str());
    throw std::runtime_error("HistCollection key too long");
  }
  const char * ch(&key[0]);
  const char * chE(ch+key.size());
  bool has_alpha(false);
  for (;ch!=chE;++ch) {
    const char c = *ch;
    if (c>='A'&&c<='z'&&(c>='a'||c<='Z')) {
      has_alpha=true;
      continue;
    }
    if (c>='-'&&c<='9')
      continue;//  This allows -./0123456789
    if (c=='+'||c=='_'||c=='%'||c==':'||c=='=')
      continue;
    printf("HistCollection ERROR key has illegal character \"%s\" (offending character is \"%c\"\n",
           key.c_str(),c);
    throw std::runtime_error("HistCollection illegal character in key");
  }

  if (!has_alpha) {
    printf("HistCollection ERROR key has no alphabetic characters (a-zA_Z): \"%s\"\n",
           key.c_str());
    throw std::runtime_error("HistCollection no alphabetic character in key");
  }

  if (Core::ends_with(key,".shist")) {
    printf("HistCollection ERROR key ends with .shist: \"%s\"\n",key.c_str());
    throw std::runtime_error("HistCollection key ends with .shist");
  }

  //All ok, just need to check for duplicates.
  auto it = m_hists.find(key);
  if (it!=m_hists.end()) {
    printf("HistCollection ERROR duplicate key \"%s\"\n",key.c_str());
    throw std::runtime_error("HistCollection duplicate key");
  }
}

bool SimpleHists::HistCollection::hasKey(const std::string& key) const
{
  return m_hists.find(key)!=m_hists.end();
}

const SimpleHists::HistBase* SimpleHists::HistCollection::hist(const std::string& key) const
{
  auto it = m_hists.find(key);
  if (it==m_hists.end()) {
    printf("HistCollection::hist ERROR unknown key \"%s\"\n",key.c_str());
    throw std::runtime_error("HistCollection::hist unknown key");
  }
  return it->second;
}

//Expensive search for key associated to histogram (returns empty string if not part of this collection)
const std::string& SimpleHists::HistCollection::getKey(const HistBase*h) const
{
  //Expensive reverse search!
  auto itE = m_hists.end();
  for (auto it = m_hists.begin(); it!=itE; ++it) {
    if (it->second == h)
      return it->first;
  }
  static std::string empty;
  return empty;
}

SimpleHists::HistBase* SimpleHists::HistCollection::hist(const std::string& key)
{
  return const_cast<HistBase*>(const_cast<const HistCollection*>(this)->hist(key));
}

void SimpleHists::HistCollection::saveToFile(const std::string& filename, bool allowOverwrite) const
{
  //File format is a magic 4 byte word (0x51415709) (~="sihistog"), followed by
  //the version (4 bytes), the number of histograms (4 bytes) and then finally
  //the histograms one by one.
  //
  //Each histogram consists of the key (1 byte for length and then the key
  //content) and the histogram itself (4 bytes for the length and then the content).
  //
  //Everything is compressed with zlib, so the magic word is actually not the
  //first four bytes of the on-disk format.

  // 1) Check filename for overwriting and extension:

  std::string fn = filename;
  if (!Core::ends_with(fn,".shist"))
    fn += ".shist";

  if ( !allowOverwrite && Core::file_exists(fn)) {
    printf("HistCollection::saveToFile ERROR file exists \"%s\" and overwriting was not allowed\n",fn.c_str());
    throw std::runtime_error("HistCollection::saveToFile file exists and overwriting was not allowed.");
  }

  //2) Write 12 byte header

  gzFile fp = gzopen(fn.c_str(),"wb9");//compression level 9
  if (!fp) {
    printf("HistCollection::saveToFile ERROR could not open file \"%s\"\n",fn.c_str());
    throw std::runtime_error("HistCollection::saveToFile could not open file");
  }

  std::uint32_t header[3];
  header[0] = (std::uint32_t)MAGICWORD;
  header[1] = (std::uint32_t)1;//version
  header[2] = (std::uint32_t)m_hists.size();

  int written = gzwrite ( fp, &header[0], 12);
  if (written != 12) {
    printf("HistCollection::saveToFile ERROR problems writing header to file \"%s\"\n",fn.c_str());
    throw std::runtime_error("HistCollection::saveToFile problems writing header");
  }

  //3) Write histogram (and key) data:

  //Each histogram consists of the key (1 byte for length and then the key
  //content) and the histogram itself (4 bytes for the length and then the content).

  char histheader[65];//remember we don't allow keys to be longer than 60
  std::string tmp;

  auto itE = m_hists.end();
  for (auto it = m_hists.begin(); it!=itE; ++it)
    {
      assert(it->first.size()<=60);
      tmp.clear();
      it->second->serialise(tmp);
      //fill data:
      std::uint32_t lenhistdata = tmp.size();
      //fill header:
      histheader[0]=static_cast<char>(it->first.size());
      std::memcpy(&histheader[1], &lenhistdata, 4);
      std::memcpy(&histheader[5], it->first.c_str(),it->first.size());
      //write:
      bool write_ok(true);
      written = gzwrite(fp, &histheader[0], 5+it->first.size());
      if(written<=0||(size_t)written != 5+it->first.size()) {
        write_ok = false;
      } else {
        written = gzwrite(fp, &tmp[0], tmp.size());
        if (written<=0||(size_t)written != tmp.size())
          write_ok = false;
      }
      if (!write_ok)
        throw std::runtime_error("HistCollection::saveToFile problems writing histogram data");
    }

  int res = gzclose(fp);
  if(res!=Z_OK)
    throw std::runtime_error("HistCollection::saveToFile problems closing file");

}


SimpleHists::HistCollection::HistCollection(const std::string& filename)
{
  // 1) Check filename for extension and that it exists:

  std::string fn = filename;
  if (!Core::ends_with(fn,".shist"))
    fn += ".shist";

  if ( !Core::file_exists(fn)) {
    printf("HistCollection ERROR shist file not found: \"%s\"\n",fn.c_str());
    throw std::runtime_error("HistCollection shist file not found.");
    return;
  }

  //2) Read uncompressed 12 byte header

  gzFile fp = gzopen(fn.c_str(),"rb");
  if (!fp) {
    printf("HistCollection ERROR could not open file \"%s\"\n",fn.c_str());
    throw std::runtime_error("HistCollection could not open file");
    return;
  }

  std::uint32_t header[3];
  int bytesread = gzread (fp, (char*)&header[0], 12);

  if (bytesread!=12||header[0]!=(std::uint32_t)MAGICWORD) {
    printf("HistCollection ERROR shist file header not in right format: \"%s\"\n",fn.c_str());
    throw std::runtime_error("HistCollection shist file header not in right format.");
    return;
  }
  if (header[1]!=1) {
    printf("HistCollection ERROR shist file \"%s\" has unsupported version %i: \n",fn.c_str(),(int)header[1]);
    throw std::runtime_error("HistCollection shist file version not supported.");
    return;
  }

  //3) Read histogram (and key) data:

  char keysize;
  std::uint32_t histsize;
  std::string keybuf;
  std::string histbuf;

  unsigned nhists = header[2];
  for (unsigned i=0;i<nhists;++i) {
    bytesread = gzread (fp, &keysize, 1);
    if (bytesread!=1)
      throw std::runtime_error("HistCollection problems reading histogram header (1)");
    bytesread = gzread (fp, (char*)&histsize, 4);
    if (bytesread!=4)
      throw std::runtime_error("HistCollection problems reading histogram header (2)");
    keybuf.resize(keysize);
    bytesread = gzread (fp, &keybuf[0], keysize);
    if (bytesread!=keysize)
      throw std::runtime_error("HistCollection problems reading histogram header (3)");
    histbuf.resize(histsize);
    bytesread = gzread (fp, &histbuf[0], histsize);
    if (bytesread!=static_cast<int>(histsize))
      throw std::runtime_error("HistCollection problems reading histogram data");
    auto it = m_hists.find(keybuf);
    if (it!=m_hists.end())
      throw std::runtime_error("HistCollection ERROR duplicate key in file!");
    HistBase * h = deserialise(histbuf);
    if (!h)
      throw std::runtime_error("HistCollection ERROR problems unpacking histogram!");
    m_hists[keybuf]=h;
  }

  int res = gzclose(fp);
  if(res!=Z_OK)
    throw std::runtime_error("HistCollection ERROR problems closing file after deserialisation");
}

void SimpleHists::HistCollection::getKeys(std::set<std::string>& keys) const
{
  auto itE=m_hists.end();
  for (auto it=m_hists.begin();it!=itE;++it)
    keys.insert(it->first);
}


void SimpleHists::HistCollection::merge(const std::string& filename_other_collection)
{
  auto o = new HistCollection(filename_other_collection);
  merge(o);
  delete o;
}

void SimpleHists::HistCollection::merge(const HistCollection* o)
{
  assert(o);

  //Check for compatibility:
  const char * e = "HistCollection attempted merge with incompatible collection";
  if (m_hists.size()!=o->m_hists.size())
    throw std::runtime_error(e);
  auto it=m_hists.begin();
  auto itE=m_hists.end();
  auto ito=o->m_hists.begin();
  for (;it!=itE;++it,++ito) {
    if (it->first!=ito->first)
      throw std::runtime_error(e);
    if (!it->second->mergeCompatible(ito->second))
      throw std::runtime_error(e);
  }
  assert(ito==o->m_hists.end());

  //Merge:
  it=m_hists.begin();
  ito=o->m_hists.begin();
  for (;it!=itE;++it,++ito)
    it->second->merge(ito->second);
  assert(ito==o->m_hists.end());
}

bool SimpleHists::HistCollection::isSimilar(const HistCollection*o) const
{
  assert(o);

  //First some quick checks:
  if (m_hists.size()!=o->m_hists.size())
    return false;

  auto it=m_hists.begin();
  auto itE=m_hists.end();
  auto ito=o->m_hists.begin();
  for (;it!=itE;++it,++ito) {
    if (it->first!=ito->first)
      return false;
  }

  //Ok, same number of histograms and same keys. Now check contents.
  it=m_hists.begin();
  ito=o->m_hists.begin();
  for (;it!=itE;++it,++ito) {
    if (!it->second->isSimilar(ito->second))
      return false;
  }
  return true;
}

void SimpleHists::HistCollection::add(HistBase*h,const std::string&key)
{
  testKey(key);
  m_hists[key]=h;
}

SimpleHists::HistBase* SimpleHists::HistCollection::remove(const std::string&key)
{
  auto it = m_hists.find(key);
  if (it==m_hists.end()) {
    printf("HistCollection::remove ERROR unknown key \"%s\"\n",key.c_str());
    throw std::runtime_error("HistCollection::remove unknown key");
  }
  HistBase* h = it->second;
  m_hists.erase(it);
  return h;
}
