#ifndef __SCAG_UTIL_STORAGE_GLOSSARY_H__
#define __SCAG_UTIL_STORAGE_GLOSSARY_H__

#include <string>
#include <vector>

#include <logger/Logger.h>
#include <core/buffers/File.hpp>
#include <core/buffers/Hash.hpp>

#include "GlossaryBase.h"

namespace scag { namespace util { namespace storage {

using std::string;
using std::vector;
using smsc::core::buffers::Hash;
using smsc::logger::Logger;

class Glossary: public GlossaryBase {
public:
  Glossary() : glossFile(0), opened(false), currentIndex(0), logger(Logger::getInstance("Pers.Gloss")) {};
  ~Glossary() { Close(); };
  int Open(const string& glossFileName);
  void Close();
  int Add(const string& key);
  int GetValueByKey(const string& key);
  int GetKeyByValue(int value, string& key);
	
private:
  typedef vector<string> GlossaryVector;
  typedef Hash<int> GlossaryHash;

private:
  int LoadGlossary(void);

private:
  Logger* logger;
  File* glossFile;
  GlossaryHash glossHash;
  GlossaryVector glossVector;
  int currentIndex;
  bool opened;
};

}//storage
}//util
}//scag

#endif
