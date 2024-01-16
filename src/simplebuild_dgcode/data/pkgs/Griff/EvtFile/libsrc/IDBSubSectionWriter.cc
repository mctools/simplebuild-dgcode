#include "EvtFile/IDBSubSectionWriter.hh"

EvtFile::IDBSubSectionWriter::IDBSubSectionWriter(EvtFile::FileWriter& fw)
{
  fw.registerPreFlushCallback(*this);
}

void EvtFile::IDBSubSectionWriter::aboutToFlushEventToDisk(FileWriter& fw)
{
  if (this->needsWrite()) {
    fw.writeDataDBSection(this->uniqueSubSectionID());
    this->write(fw);
  }
}
