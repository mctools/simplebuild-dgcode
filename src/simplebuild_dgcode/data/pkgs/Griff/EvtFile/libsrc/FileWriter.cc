#include "EvtFile/FileWriter.hh"
#include "Core/String.hh"
#include "EvtFileDefs.hh"
#include "Utils/ProgressiveHash.hh"
#include "ZLibUtils/Compress.hh"
#include <stdexcept>

namespace EvtFile {

  FileWriter::FileWriter( const IFormat* format,
                          const char* filename,
                          int buffer_len )
    : m_format(format),
      m_buf(buffer_len ? new char[buffer_len] : nullptr),
      m_filename(filename)
  {
    if ( buffer_len > 0 )
      m_os.rdbuf()->pubsetbuf(m_buf, buffer_len );

    if (Core::ends_with(filename,format->fileExtension()))
      m_os.open(filename, std::ios::out | std::ios::binary);
    else
      m_os.open((std::string(filename)+format->fileExtension()).c_str(), std::ios::out | std::ios::binary);

    write(format->magicWord());
    write(EVTFILE_VERSION);

    m_section_database.reserve(4096);
    m_section_briefdata.reserve(4096);
    m_section_fulldata.reserve(4096);
    m_section_fulldata_compressed.reserve(4096);
  }

  FileWriter::~FileWriter()
  {
    if (is_open())
      close();
  }

  void FileWriter::close()
  {
    m_os.close();
    delete[] m_buf;
  }

  void FileWriter::flushEventToDisk(int32_t runnumber, int32_t eventnumber)
  {
    assert(is_open() && "Attempt to write to a file which is not open");
    assert(!bad() && "Attempt to write to a file with bad status");

    //First trigger callbacks:
    for( auto it=m_preFlushCBs.begin(), itE=m_preFlushCBs.end(); it!=itE; ++it )
      (*it)->aboutToFlushEventToDisk(*this);

    bool compress_full_data(m_format->compressFullData());

    //Compress the full data section:
    unsigned fulldata_compressed_size(0);
    if (compress_full_data&&!m_section_fulldata.empty()) {
      ZLibUtils::compressToBuffer(m_section_fulldata.data(), m_section_fulldata.size(),
                                  m_section_fulldata_compressed, fulldata_compressed_size);
    }

    //For efficient hash calculation and file i/o, put the event header in an array:
    std::uint32_t eventheader[6];
    static_assert(EVTFILE_EVENT_HEADER_BYTES==6*sizeof(std::uint32_t));

    //The first field in the header is reserved for the hash:
    eventheader[1] = runnumber;
    eventheader[2] = eventnumber;
    eventheader[3] = (std::uint32_t)m_section_database.size();
    eventheader[4] = (std::uint32_t)m_section_briefdata.size();
    if (compress_full_data)
      eventheader[5] = (std::uint32_t)(fulldata_compressed_size);
    else
      eventheader[5] = (std::uint32_t)m_section_fulldata.size();

    //Calculate the hash (from uncompressed data!):
    ProgressiveHash hash;
    hash.addData((char*)&(eventheader[1]),5*sizeof(std::uint32_t));
    if (!m_section_database.empty()) hash.addData(m_section_database.data(),m_section_database.size());
    if (!m_section_briefdata.empty()) hash.addData(m_section_briefdata.data(),m_section_briefdata.size());
    if (!m_section_fulldata.empty()) hash.addData(m_section_fulldata.data(),m_section_fulldata.size());
    eventheader[0] = hash.getHash();

    //Write out the header:
    write((char*)&(eventheader[0]),6*sizeof(std::uint32_t));

    //Write out the three data blobs:
    if (!m_section_database.empty()) write(m_section_database);
    if (!m_section_briefdata.empty()) write(m_section_briefdata);
    if (!m_section_fulldata.empty()) {
      if (compress_full_data)
        write(m_section_fulldata_compressed);
      else
        write(m_section_fulldata);
    }

    if (!m_os.good()) {
      printf("EvtFile ERROR: Troubles encountered while writing to file %s\n",m_filename.c_str());
      printf("               => File might be corrupted!\n");
      throw std::runtime_error("Data file write failed");
    }
    m_section_database.clear();
    m_section_briefdata.clear();
    m_section_fulldata.clear();
    m_section_fulldata_compressed.clear();
  }

}


