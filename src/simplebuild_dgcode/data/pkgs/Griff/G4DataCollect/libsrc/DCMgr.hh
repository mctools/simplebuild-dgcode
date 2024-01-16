#ifndef G4DataCollect_DCMgr_hh
#define G4DataCollect_DCMgr_hh

//Manager class which keeps both the FileWriter and the various
//IDBSubSectionWriter's needed to write out the G4 data.

#include "EvtFile/FileWriter.hh"
#include "EvtFile/DBEntryWriter.hh"
#include "EvtFile/DBStringsWriter.hh"
#include "GriffFormat/Format.hh"
#include "PDGCodeWriter.hh"

#include "G4RunManager.hh"
#include "G4Run.hh"
#include "G4Event.hh"

namespace G4DataCollectInternals {


  struct DCMgr {
    DCMgr(const char* outputFile)
      : fileWriter(GriffFormat::Format::getFormat(),outputFile),
        dbTouchables(GriffFormat::Format::subsectid_touchables,fileWriter),
        dbVolNames(GriffFormat::Format::subsectid_volnames,fileWriter),
        dbMaterials(GriffFormat::Format::subsectid_materials,fileWriter),
        dbElements(GriffFormat::Format::subsectid_elements,fileWriter),
        dbIsotopes(GriffFormat::Format::subsectid_isotopes,fileWriter),
        dbMaterialNames(GriffFormat::Format::subsectid_materialnames,fileWriter),
        dbElementNames(GriffFormat::Format::subsectid_elementnames,fileWriter),
        dbIsotopeNames(GriffFormat::Format::subsectid_isotopenames,fileWriter),
        dbProcNames(GriffFormat::Format::subsectid_procnames,fileWriter),
        dbPDGCodes(GriffFormat::Format::subsectid_pdgcodes,fileWriter,this),
        dbPDGNames(GriffFormat::Format::subsectid_pdgnames,fileWriter),
        dbPDGTypes(GriffFormat::Format::subsectid_pdgtypes,fileWriter),
        dbPDGSubTypes(GriffFormat::Format::subsectid_pdgsubtypes,fileWriter),
        dbMetaData(GriffFormat::Format::subsectid_metadata,fileWriter),
        dbMetaDataStrings(GriffFormat::Format::subsectid_metadatastrings,fileWriter)
    {
    }
    ~DCMgr(){}

    EvtFile::FileWriter fileWriter;
    EvtFile::DBEntryWriter dbTouchables;
    EvtFile::DBStringsWriter dbVolNames;//filled during dbTouchables write stage, so must come after
    EvtFile::DBEntryWriter dbMaterials;//must be after dbTouchables
    EvtFile::DBEntryWriter dbElements;//must be after dbMaterials
    EvtFile::DBEntryWriter dbIsotopes;//must be after dbElements
    EvtFile::DBStringsWriter dbMaterialNames;//must be after dbMaterials
    EvtFile::DBStringsWriter dbElementNames;//must be after dbElements
    EvtFile::DBStringsWriter dbIsotopeNames;//must be after dbIsotopes
    EvtFile::DBStringsWriter dbProcNames;
    PDGCodeWriter dbPDGCodes;
    EvtFile::DBStringsWriter dbPDGNames;//filled during dbPDGCodes write stage, so must come after
    EvtFile::DBStringsWriter dbPDGTypes;//filled during dbPDGCodes write stage, so must come after
    EvtFile::DBStringsWriter dbPDGSubTypes;//filled during dbPDGCodes write stage, so must come after
    EvtFile::DBEntryWriter dbMetaData;
    EvtFile::DBStringsWriter dbMetaDataStrings;//must be after dbMetaData;

    void flushEventToDisk()
    {
      unsigned runid = G4RunManager::GetRunManager()->GetCurrentRun()->GetRunID();
      unsigned evtid = G4RunManager::GetRunManager()->GetCurrentEvent()->GetEventID();
      fileWriter.flushEventToDisk(runid,evtid);
    }
  private:
    DCMgr( const DCMgr & );
    DCMgr & operator= ( const DCMgr & );
  };

}

#endif
