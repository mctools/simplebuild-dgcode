#ifndef EvtFile_FileWriter_hh
#define EvtFile_FileWriter_hh

//Class responsible for actual writing of the data files. Normal users should
//not have a need to use this file.

#include "EvtFile/IFormat.hh"
#include "Core/Types.hh"
#include <cassert>
#include <vector>
#include <fstream>
#include <cstring>
#include "Utils/DynBuffer.hh"

//TODO: Add method which will ignore current event.
//TODO: Make sure (#evt,#run) is unique in file (merger scripts must honour this)
//TODO: Add bad_reason() method like on Reader

namespace EvtFile {

  class FileWriter;

  struct IFWPreFlushCB {
    virtual void aboutToFlushEventToDisk(FileWriter&) = 0;
  };

  class FileWriter final {
  public:
    ////////////////////////////////////////////
    //  Methods for opening/closing the file  //
    ////////////////////////////////////////////

    //The constructor opens the file and writes the first few words identifying
    //the filetype. It will append the appropriate file extension of the format
    //to the filename if it doesn't have that ending already.
    FileWriter( const IFormat*,
                const char* filename,
                int buffer_len=8192 );

    FileWriter( const FileWriter& ) = delete;
    FileWriter& operator=( const FileWriter& ) = delete;
    FileWriter( FileWriter&& ) = delete;
    FileWriter& operator=( FileWriter&& ) = delete;

    //Register callbacks to be notified just before events are flush to disk:
    void registerPreFlushCallback(IFWPreFlushCB&);

    //File should be open after the constructor was run unless an error
    //occured. It will also cease to be considered open after a call to close():
    bool is_open() const { return m_os.is_open(); }

    bool bad() const { return m_os.bad(); }

    bool ok() const { return is_open() && !bad(); }

    //Call to close the file:
    void close();

    //The destructor will call close() if the file is still open:
    ~FileWriter();

    ////////////////////////////////////////////
    //  Methods used to write out each event  //
    ////////////////////////////////////////////

    void writeDataDBSection(const char* data, unsigned nbytes);
    void writeDataBriefSection(const char* data, unsigned nbytes);
    void writeDataFullSection(const char* data, unsigned nbytes);

    template<class T>
    void writeDataDBSection(const T& data) { writeDataDBSection(reinterpret_cast<const char*>(&data), sizeof(T)); }
    template<class T>
    void writeDataBriefSection(const T& data) { writeDataBriefSection(reinterpret_cast<const char*>(&data), sizeof(T)); }
    template<class T>
    void writeDataFullSection(const T& data) { writeDataFullSection(reinterpret_cast<const char*>(&data), sizeof(T)); }

    //The index in number of bytes in each section where the next data will be written:
    unsigned sizeDBSection() const { return m_section_database.size(); }
    unsigned sizeBriefDataSection() const { return m_section_briefdata.size(); }
    unsigned sizeFullDataSection() const { return m_section_fulldata.size(); }

    void flushEventToDisk(int32_t runnumber, int32_t eventnumber);//Call at end of each event

  private:

    // FileWriter( const FileWriter & );
    // FileWriter & operator= ( const FileWriter & );

    const IFormat* m_format;
    char * m_buf;
    std::ofstream m_os;
    Utils::DynBuffer<char> m_section_database;
    Utils::DynBuffer<char> m_section_briefdata;
    Utils::DynBuffer<char> m_section_fulldata;
    Utils::DynBuffer<char> m_section_fulldata_compressed;
    std::vector<IFWPreFlushCB*> m_preFlushCBs;
    std::string m_filename;
    void write( const char*data, unsigned nbytes ) { m_os.write( data, nbytes); }
    void write( const Utils::DynBuffer<char>& buf )
    { if (!buf.empty())
        m_os.write( buf.data(), buf.size());
    }
    template<class T>
    void write(const T&t) { m_os.write( (char*)&t, sizeof(t)); }
  };

#include "FileWriter.icc"

}

#endif
