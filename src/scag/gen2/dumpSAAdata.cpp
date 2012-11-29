/*
 * dumpSAAdata.cpp
 *
 *  Created on: 24.09.2012
 *      Author: opa
 *
 *  Dump msag's v2/v3 statistics/saa/events.*
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <exception>

#include "logger/Logger.h"

#include "scag/util/SerializeBuffer.h"
#include <inttypes.h>
#include "scag/stat/base/Statistics2.h"
//#include <core/buffers/File.hpp>

//#include "informer/io/FileGuard.h"
#include "informer/io/FileReader.h"
#include "informer/io/IOConverter.h"  //class FromBuf


using namespace smsc::logger;
using namespace eyeline::informer;

class SAARecordReader:public FileReader::RecordReader{
  static const size_t sizeofRecLen=sizeof(uint32_t);
public:
  virtual bool isStopping() {
      return false;
  }

  /// return the size of record length in octets.
  virtual size_t recordLengthSize() const { return sizeofRecLen; }

  /// read record length from fb and checks its validity.
  virtual size_t readRecordLength( size_t filePos, char* buf, size_t buflen )
  {
      const size_t rl(FromBuf(buf,buflen).get32()); //recordSizeSize
      if (rl>10000){
          throw InfosmeException(EXC_BADFILE,"reclen=%u is too big (>10000)",unsigned(rl));
      }
      return rl-sizeofRecLen;
  }

  /// read the record data (w/o length)
  virtual bool readRecordData( size_t filePos, char* buf, size_t buflen )
  {
    std::auto_ptr<scag2::stat::SaccEvent> event(scag2::stat::SaccEvent::deserialize(buf,buflen));
      //SaccEvent (SaccTrafficInfoEvent or SaccBillingInfoEvent)
    if(!event.get())
      throw eyeline::informer::InfosmeException(eyeline::informer::EXC_BADFORMAT, "Invalid SaccEvent type");
    //dump_to_stdout(event);
    if(mode_ == scag2::stat::SaccEvent::JSON) printf("dump a %s\n",event->getName());
    printf("%s\n",event->toString(mode_).c_str());
    return true;
  }
  virtual ~SAARecordReader(){}
  SAARecordReader(scag2::stat::SaccEvent::tostring mode=scag2::stat::SaccEvent::JSON):mode_(mode){}
private:
  scag2::stat::SaccEvent::tostring mode_;
};


void dumpFile(const char* fileName,scag2::stat::SaccEvent::tostring mode){
  Logger *logger = Logger::getInstance("dumpSAAdata");
  printf("dump SAA file <%s>\n", fileName);
  try {
    TmpBuf<char,8192> buf;
    FileGuard fg;
    fg.ropen(fileName);
    size_t recread = 0;
    FileReader fileReader(fg);
    SAARecordReader recordReader(mode);
    recread = fileReader.readRecords(buf,recordReader);
  //} catch ( FileReadException& e ) {
  } catch (std::exception& e) {
    fprintf(stderr, "Read error at SAA file <%s>: %s\n", fileName, e.what());
  }
}

void help(char* argv0, bool dumpToStderr){
  FILE* f=dumpToStderr?stderr:stdout;
  fprintf(f,"Dump SAA datafile from msag v2/v3 to stdout.\n");
  fprintf(f,"call:\n");
  fprintf(f,"%s --help  -- to get this message\n", argv0);
  fprintf(f,"%s [-d|--debug] [-j|--JSON | -c|--CSV] {filename }+  -- dump this files\n", argv0);
  fprintf(f,"  -d, --debug -- output debug messages to stderr\n");
  fprintf(f,"  -j, --JSON -- output dump in JSON-like format\n");
  fprintf(f,"  -c, --CSV  -- output dump in  CSV-like format\n");
}

int main(int argc, char* argv[]){
  if(argc==1 || !strcasecmp(argv[1],"--help")){
    help(argv[0], argc==1);
    return 0;
  }

  if(argc && (!strcasecmp(argv[1],"-d")||!strcasecmp(argv[1],"--debug"))){
    Logger::initForTest(Logger::LEVEL_DEBUG);
    argc--; argv++; //shift arglist
  }else
    Logger::initForTest(Logger::LEVEL_INFO);

  scag2::stat::SaccEvent::tostring mode=scag2::stat::SaccEvent::CSV;
  if(argc && (!strcasecmp(argv[1],"-j")||!strcasecmp(argv[1],"--JSON"))){
    mode=scag2::stat::SaccEvent::JSON;
    argc--; argv++; //shift arglist
  }
  if(argc && (!strcasecmp(argv[1],"-c")||!strcasecmp(argv[1],"--CSV"))){
    mode=scag2::stat::SaccEvent::CSV;
    argc--; argv++; //shift arglist
  }

  for(int i=1; i<argc; i++)
    dumpFile(argv[i],mode);

  Logger::Shutdown();
  return 0;
}



