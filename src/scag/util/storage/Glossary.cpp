#include "Glossary.h"

namespace scag { namespace util { namespace storage {
	
int Glossary::Open(const string& glossFileName) {
  if (opened) {
    return ALREADY_OPENED;
  }
  smsc_log_debug(logger, "open");
  currentIndex = 0;
  glossVector.reserve(1024);

  if (!glossFile) {
    glossFile = new File();
  }

  try {
    if (File::Exists(glossFileName.c_str())) {
      glossFile->RWOpen(glossFileName.c_str());
      LoadGlossary();
    } else {
      glossFile->RWCreate(glossFileName.c_str());
    }
    glossFile->SetUnbuffered();
  } catch(const FileException& ex) {
    smsc_log_debug(logger, "error open or create Glossary File - %s", ex.what());
    return OPEN_ERROR;
  }
  opened = true;
  return SUCCESS;
}

void Glossary::Close() {
  smsc_log_debug(logger, "close");
  if (!opened) {
    smsc_log_debug(logger, "cancel close - Glossary not opened");
    return;
  }
  glossHash.Empty();

  glossVector.erase(glossVector.begin(), glossVector.end());
  glossFile->Close();
  delete glossFile;
  glossFile = NULL;
  opened=false;
}

int Glossary::Add(const string& key) {
  smsc_log_debug(logger, "add %s", key.c_str());
  if (!opened) {
    smsc_log_debug(logger, "cancel add - Glossary not opened");
    return GLOSSARY_CLOSED;
  }
  if (!opened) {
    return GLOSSARY_CLOSED;
  }

  if (!glossHash.Exists(key.c_str())) {
    char ch = 0x0a;
    glossFile->Write(key.c_str(), key.length());
    glossFile->Write(&ch, sizeof(char)); 
    glossHash.Insert(key.c_str(), currentIndex);
    glossVector.push_back(key);
    currentIndex++;
    smsc_log_debug(logger, "added %s - %d", key.c_str(), currentIndex - 1);
    return currentIndex - 1;
  }
  smsc_log_debug(logger, "added canceled - key %s exists", key.c_str());
  return KEY_ALREADY_EXISTS;
}

int Glossary::GetValueByKey(const string& key) {
  // smsc_log_debug(logger, "GetValueByKey %s", key.c_str());
  if (!opened)  {
    smsc_log_debug(logger, "cancel GetValueByKey - Glossary not opened");
    return GLOSSARY_CLOSED;
  }
  const int *val = glossHash.GetPtr(key.c_str());
  if (!val) {
    smsc_log_debug(logger, "GetValueByKey. No value for %s", key.c_str());
    return NO_VALUE;
  }
  // smsc_log_debug(logger, "GetValueByKey %s - %d", key.c_str(), *val);
  return *val;
}

int Glossary::GetKeyByValue(int value, string& key) {
  // smsc_log_debug(logger, "GetKeyByValue %d", value);
  if (!opened) {
    smsc_log_debug(logger, "cancel GetKeyByValue - Glossary not opened");
    return GLOSSARY_CLOSED;
  }
  if (size_t(value) < glossVector.size()) {
    key = glossVector[value];
    smsc_log_debug(logger, "GetKeyByValue. %d - %s", value, key.c_str());
    return SUCCESS;
  }
  // smsc_log_debug(logger, "GetKeyByValue. No key for %d", value);
  return NO_KEY;
}

int Glossary::LoadGlossary(void) {
  string	key;
  smsc_log_debug(logger, "Load Glossary");
  try {
    while(glossFile->ReadLine(key)) {
      smsc_log_debug(logger, "%s %d", key.c_str(), currentIndex);
      glossHash.Insert(key.c_str(), currentIndex);
      glossVector.push_back(key);
      currentIndex++;
    }
    smsc_log_debug(logger, "Glossary size is %d (vec = %d, idx = %d)", glossHash.GetCount(), glossVector.size(), currentIndex);
    glossFile->SeekEnd(0);
  } catch(FileException ex) {
    smsc_log_debug(logger, "Load Glossary Error - %s", ex.what());
    return LOAD_ERROR;
  }
  return SUCCESS;
}

}//stoarge
}//util
}//scag
