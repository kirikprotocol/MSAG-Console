#ifdef __DBENTITYSTORAGE_DISKDATASTORAGE_HPP__
// inclusion via .hpp (templates only)
# define __DBENTITYSTORAGE_DISKDATASTORAGE_CPP__ 0
#else
# ifndef TEMPINST
// library compilation w/ templates (i.e. everything)
#  define __DBENTITYSTORAGE_DISKDATASTORAGE_CPP__ 1
# else
// library compilation w/o templates
#  define __DBENTITYSTORAGE_DISKDATASTORAGE_CPP__ 2
# endif
#endif

#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <inttypes.h>
#include <errno.h>
#include <unistd.h>

#include "DiskDataStorage.hpp"
#include <util/BufferSerialization.hpp>
#include <util/crc32.h>

#include <iomanip>
#include <ios>
#include <sstream>
#include <iostream>

//#include "LinearIOPageDispatcher.hpp"
#include "CyclicIOPageDispatcher.hpp"

#if __DBENTITYSTORAGE_DISKDATASTORAGE_CPP__ > 0
// for debug
std::string
hexdmp(uchar_t* buf, uint32_t bufSz)
{
  std::ostringstream hexBuf;
  hexBuf.fill('0');
  hexBuf << std::hex;
  for (size_t i=0; i<bufSz; ++i)
    hexBuf << std::setw(2) << (uint32_t) buf[i];

  return hexBuf.str();
}
#endif


// end for debug
#if __DBENTITYSTORAGE_DISKDATASTORAGE_CPP__ < 2

template <class V>
int
SimpleFileDispatcher<V>::readNbytesFromFile(IOPage& ioPage, uchar_t* buf, size_t bufSz)
{
  size_t sz=0;

  ssize_t st=ioPage.read(buf, bufSz);
  if ( st < 0 )
    return st;

  sz = st;
  while ( sz != bufSz ) {
    ioPage = _ioPageDispatcher->getNextIOPage(ioPage);
    st = ioPage.read(buf+sz, bufSz-sz);
    if ( st < 0 )
      return st;
    sz += st;
  }
  return 0;
}

template <class V>
int
SimpleFileDispatcher<V>::writeNbytesToFile(IOPage& ioPage, uchar_t* buf, size_t bufSz)
{
  size_t sz=0;

  ssize_t st=ioPage.write(buf, bufSz);
  if ( st < 0 )
    return st;

  smsc_log_debug(_logger, "SimpleFileDispatcher<V>::writeNbytesToFile::: wrote data to page, buf=[%s],st=%d",hexdmp(buf, st).c_str(),st);
  sz = st;
  ioPage.commitMemory();
  // ������ ����� �� ������ ������� � ��������, �������
  // ��������������� ������ � ��������� ���������������� �������
  while ( sz != bufSz ) {
    if ( !_ioPageDispatcher->haveNextPage(ioPage) )
      ioPage = _ioPageDispatcher->createNewIOPage();
    else
      ioPage = _ioPageDispatcher->getNextIOPage(ioPage);

    st = ioPage.write(buf+sz, bufSz-sz);
    if ( st < 0 )
      return st;
    smsc_log_debug(_logger, "SimpleFileDispatcher<V>::writeNbytesToFile::: page boundary condition: wrote data to next page, buf=[%s],st=%d,bufSz-sz=%d",hexdmp(buf+sz, st).c_str(),st,bufSz-sz);
    sz += st;
    ioPage.commitMemory();
  }
  return 0;
}

template<typename V>
uchar_t
SimpleFileDispatcher<V>::end_storage_marker_constant[STORAGE_END_MARKER_SZ] = { 0xAA, 0x11, 0xEE, 0x33 };

template<typename V>
uchar_t
SimpleFileDispatcher<V>::record_marker_constant[RECORD_MARKER_SZ] = { 0x00, 0x00 };

template <typename V>
DataStorage_FileDispatcher<V>::DataStorage_FileDispatcher(const std::string& fileName)
  : _storageFileName(fileName) {}

template <typename V>
SimpleFileDispatcher<V>::~SimpleFileDispatcher()
{ 
  close();
  delete _ioPageDispatcher;
}

template <class V>
typename DataStorage_FileDispatcher<V>::operation_status_t
SimpleFileDispatcher<V>::create()
{
  if ( (_fd = ::open(DataStorage_FileDispatcher<V>::_storageFileName.c_str(), O_RDWR | O_CREAT | O_EXCL, 0640)) < 0 ) {
    if ( errno == EEXIST )
      return DataStorage_FileDispatcher<V>::STORAGE_ALREADY_EXISTS;
    else
      return DataStorage_FileDispatcher<V>::FATAL;
  }
  const size_t EMPTYSTORAGEBUFSZ
    = sizeof(uint32_t) + sizeof(uint32_t) + sizeof(end_storage_marker_constant);

  smsc::util::SerializationBuffer buf(EMPTYSTORAGEBUFSZ);

  buf.WriteNetInt32(STORAGE_VERSION);
  buf.WriteNetInt32(0); // reserved bytes
  buf.Write(end_storage_marker_constant,sizeof(end_storage_marker_constant));

  if ( write(_fd, buf.getBuffer(), buf.getBufferSize()) != buf.getBufferSize() )
    return DataStorage_FileDispatcher<V>::FATAL;

  off_t offset = lseek(_fd, -sizeof(end_storage_marker_constant), SEEK_END);
  if ( offset < 0 )
    return DataStorage_FileDispatcher<V>::FATAL;

  if ( !(_ioPageDispatcher = new CyclicIOPageDispatcher(_fd)) )
    return DataStorage_FileDispatcher<V>::FATAL;

  return DataStorage_FileDispatcher<V>::OPERATION_OK;
}

template <typename V>
typename DataStorage_FileDispatcher<V>::operation_status_t
SimpleFileDispatcher<V>::open()
{
  if ( (_fd = ::open(DataStorage_FileDispatcher<V>::_storageFileName.c_str(), O_RDWR, 0)) < 0 ) {
    if ( errno == ENOENT )
      return DataStorage_FileDispatcher<V>::NO_SUCH_FILE;
    else
      return DataStorage_FileDispatcher<V>::FATAL;
  }
  const size_t EMPTYSTORAGEBUFSZ
    = sizeof(uint32_t) + sizeof(uint32_t) + sizeof(end_storage_marker_constant);

  uint8_t emptyStorageBuf[EMPTYSTORAGEBUFSZ];
  if ( read(_fd, emptyStorageBuf, sizeof(emptyStorageBuf)) != sizeof(emptyStorageBuf))
    return DataStorage_FileDispatcher<V>::FATAL;

  smsc::util::SerializationBuffer buf;
  buf.setExternalBuffer(emptyStorageBuf,EMPTYSTORAGEBUFSZ);
  if ( buf.ReadNetInt32() != STORAGE_VERSION )
    return DataStorage_FileDispatcher<V>::INVALID_FILE_VERSION;

  if ( lseek(_fd, -sizeof(end_storage_marker_constant), SEEK_CUR) < 0 ||
       !(_ioPageDispatcher = new CyclicIOPageDispatcher(_fd)) )
    return DataStorage_FileDispatcher<V>::FATAL;

  return DataStorage_FileDispatcher<V>::OPERATION_OK;
}

template <typename V>
typename DataStorage_FileDispatcher<V>::operation_status_t
SimpleFileDispatcher<V>::close()
{
  if ( _fd >= 0 ) {
    if ( ::close(_fd) < 0 )
      return DataStorage_FileDispatcher<V>::FATAL;

    _fd = -1;
  }
  return DataStorage_FileDispatcher<V>::OPERATION_OK;
}

template <typename V>
typename DataStorage_FileDispatcher<V>::operation_status_t
SimpleFileDispatcher<V>::drop()
{
  if ( close() != DataStorage_FileDispatcher<V>::OPERATION_OK ||
       unlink(DataStorage_FileDispatcher<V>::_storageFileName.c_str()) < 0 )
    return DataStorage_FileDispatcher<V>::FATAL;
  else
    return DataStorage_FileDispatcher<V>::OPERATION_OK;
}

template <typename V>
typename DataStorage_FileDispatcher<V>::operation_status_t
SimpleFileDispatcher<V>::checkEndStorageMarker(const IOPage& ioPage)
{
  uchar_t end_marker_buf[STORAGE_END_MARKER_SZ];

  IOPage dupIoPage = ioPage;
  if ( readNbytesFromFile(dupIoPage, end_marker_buf, sizeof(end_marker_buf)) < 0 )
    return DataStorage_FileDispatcher<V>::FILE_DAMAGED;

  if ( !memcmp(end_marker_buf, end_storage_marker_constant, sizeof(end_storage_marker_constant)) )
    return DataStorage_FileDispatcher<V>::NO_RECORD_FOUND;
  else
    return DataStorage_FileDispatcher<V>::OPERATION_OK;
}

template <typename V>
typename DataStorage_FileDispatcher<V>::operation_status_t
SimpleFileDispatcher<V>::extractRecord(V* record, const typename DataStorage_FileDispatcher<V>::rid_t& rid)
{
  IOPage ioPage=_ioPageDispatcher->getIOPage(rid);

  StorableRecord readbleRecord;
  operation_status_t 
    opRes = readbleRecord.unmarshal(ioPage, *this);

  if ( opRes == DataStorage_FileDispatcher<V>::OPERATION_OK )
    *record = *(readbleRecord.getReadRecord());
  return opRes;
}

template <typename V>
typename DataStorage_FileDispatcher<V>::operation_status_t
SimpleFileDispatcher<V>::addRecord(const V& record, typename DataStorage_FileDispatcher<V>::rid_t* rid)
{
  uchar_t end_marker_buf[STORAGE_END_MARKER_SZ];
  typename DataStorage_FileDispatcher<V>::rid_t addedRid;

#ifdef MAKE_ADDRECORD_LOCK
  smsc::core::synchronization::MutexGuard mutexGuard(_addRecordLock);
#endif

  IOPage ioPage=_ioPageDispatcher->getLastIOPage();

  if ( ioPage.setPosition(0, SEEK_END) < 0 )
    return DataStorage_FileDispatcher<V>::FATAL;

  ssize_t sz = ioPage.readOrderReverse(end_marker_buf, sizeof(end_marker_buf));
  if ( sz < 0 )
    return DataStorage_FileDispatcher<V>::FILE_DAMAGED;

  if ( sz != sizeof(end_marker_buf) ) {
    ioPage = _ioPageDispatcher->getPreviousIOPage(ioPage);
    if ( ioPage.setPosition(0, SEEK_END) < 0 )
      return DataStorage_FileDispatcher<V>::FATAL;
    sz = sizeof(end_marker_buf) - sz;
    if ( ioPage.readOrderReverse(end_marker_buf, sz) != sz )
      return DataStorage_FileDispatcher<V>::FATAL;
    if ( memcmp(end_marker_buf, end_storage_marker_constant, sizeof(end_storage_marker_constant)) )
      return DataStorage_FileDispatcher<V>::FILE_DAMAGED;
  }

  addedRid=ioPage.getAbsolutPosition();

  StorableRecord addingRecord(&record,(uint8_t)0);
  if ( addingRecord.marshal(ioPage, *this) )
    return DataStorage_FileDispatcher<V>::FATAL;

  if ( writeNbytesToFile(ioPage, end_storage_marker_constant, sizeof(end_storage_marker_constant)) < 0 )
    return DataStorage_FileDispatcher<V>::FATAL;

  *rid = addedRid;
  return DataStorage_FileDispatcher<V>::OPERATION_OK;
}

template <typename V>
typename DataStorage_FileDispatcher<V>::operation_status_t
SimpleFileDispatcher<V>::extractFirstRecord(V* record, typename DataStorage_FileDispatcher<V>::rid_t* rid, rid_t* nextRid)
{
  IOPage ioPage=_ioPageDispatcher->getFirstIOPage();

  StorableRecord readbleRecord;
  operation_status_t opRes;
  rid_t firstRecordRid;
  do {
    firstRecordRid = ioPage.getAbsolutPosition();
    opRes = checkEndStorageMarker(ioPage);
    if ( opRes != DataStorage_FileDispatcher<V>::OPERATION_OK )
      return opRes;
    opRes = readbleRecord.unmarshal(ioPage, *this);
  } while( opRes == DataStorage_FileDispatcher<V>::RECORD_DELETED );

  if ( opRes == DataStorage_FileDispatcher<V>::OPERATION_OK ) {
    *record = *(readbleRecord.getReadRecord());
    *rid = firstRecordRid; *nextRid = ioPage.getAbsolutPosition();
    //    smsc_log_debug(_logger, "SimpleFileDispatcher<V>::extractFirstRecord::: *rid=[%d]",*rid);
  }
  
  return opRes;
}

template <typename V>
typename DataStorage_FileDispatcher<V>::operation_status_t
SimpleFileDispatcher<V>::extractNextRecord(V* record, typename DataStorage_FileDispatcher<V>::rid_t* rid, rid_t* nextRid)
{
  IOPage ioPage=_ioPageDispatcher->getIOPage(*nextRid);

  StorableRecord readbleRecord;
  operation_status_t opRes;
  rid_t nextRecordRid;

  ioPage.setPosition(*nextRid, SEEK_SET);
  do {
    nextRecordRid = ioPage.getAbsolutPosition();
    opRes = checkEndStorageMarker(ioPage);
    if ( opRes != DataStorage_FileDispatcher<V>::OPERATION_OK )
      return opRes;

    opRes = readbleRecord.unmarshal(ioPage, *this);
  } while( opRes == DataStorage_FileDispatcher<V>::RECORD_DELETED );

  if ( opRes == DataStorage_FileDispatcher<V>::OPERATION_OK ) {
    *record = *(readbleRecord.getReadRecord());
    *rid = nextRecordRid; *nextRid = ioPage.getAbsolutPosition();
    //    smsc_log_debug(_logger, "SimpleFileDispatcher<V>::extractNextRecord::: *rid=[%d]",*rid);
  }

  return opRes;
}

template <typename V>
typename DataStorage_FileDispatcher<V>::operation_status_t
SimpleFileDispatcher<V>::deleteRecord(const typename DataStorage_FileDispatcher<V>::rid_t& rid)
{
  IOPage ioPage=_ioPageDispatcher->getIOPage(rid);

  rid_t old_rid;
  old_rid=ioPage.getAbsolutPosition();
  if ( old_rid < 0 )
    return DataStorage_FileDispatcher<V>::FATAL;

  if ( ioPage.setPosition(rid, SEEK_SET) < 0 )
    return DataStorage_FileDispatcher<V>::FATAL;

  IOPage saveIoPage=ioPage;

  operation_status_t 
    opResult = checkEndStorageMarker(ioPage);
  if ( opResult != DataStorage_FileDispatcher<V>::OPERATION_OK )
    return opResult;

  StorableRecord readableRecord;

  opResult = readableRecord.unmarshal(ioPage, *this);
  if ( opResult == DataStorage_FileDispatcher<V>::OPERATION_OK ) {
    StorableRecord writeableRecord(readableRecord.getReadRecord(),
                                   RECORD_DELETED_FLAG);
    opResult = writeableRecord.marshal(saveIoPage, *this);
  }

  return opResult;
}

template <typename V>
typename DataStorage_FileDispatcher<V>::operation_status_t
SimpleFileDispatcher<V>::replaceRecord(const V& record, const typename DataStorage_FileDispatcher<V>::rid_t& rid)
{
  IOPage ioPage=_ioPageDispatcher->getIOPage(rid);

  rid_t old_rid;
  old_rid=ioPage.getAbsolutPosition();
  if ( old_rid < 0 )
    return DataStorage_FileDispatcher<V>::FATAL;

  if (ioPage.setPosition(rid, SEEK_SET) < 0 )
    return DataStorage_FileDispatcher<V>::FATAL;

  IOPage saveIoPage=ioPage;

  operation_status_t 
    opResult = checkEndStorageMarker(ioPage);
  if ( opResult != DataStorage_FileDispatcher<V>::OPERATION_OK )
    return opResult;

  StorableRecord readableRecord;

  opResult = readableRecord.unmarshal(ioPage, *this);

  if ( opResult == DataStorage_FileDispatcher<V>::OPERATION_OK ) {
    if ( readableRecord.getReadRecord()->getSize() == record.getSize() ) {
      StorableRecord writeableRecord(&record,0);
      opResult = writeableRecord.marshal(saveIoPage, *this);
    } else
      opResult = DataStorage_FileDispatcher<V>::REPLACEMENT_NOT_ALLOWED;
  }

  return opResult;
}

template <typename V>
typename DataStorage_FileDispatcher<V>::operation_status_t
SimpleFileDispatcher<V>::StorableRecord::marshal(IOPage& ioPage, SimpleFileDispatcher<V>& fileDispatcher) {
  smsc::util::SerializationBuffer
    serialize_recordBuf(RECORD_OVERHEAD_SZ + _recordSz);

  serialize_recordBuf.Write(record_marker_constant, sizeof(record_marker_constant));

  serialize_recordBuf.WriteNetInt16(_recordSz);
  _crc = smsc::util::crc32(0,&_recordSz,sizeof(_recordSz));

  serialize_recordBuf.WriteByte(_flg);
  _crc = smsc::util::crc32(_crc,&_flg,sizeof(_flg));

  _writingAppData->marshal(&serialize_recordBuf);

  _crc = _writingAppData->calcCrc(_crc);
  serialize_recordBuf.WriteNetInt32(_crc);

  smsc_log_debug(_logger, "SimpleFileDispatcher<V>::marshal::: crc=%08X,wrote buf=[%s]",_crc,hexdmp((uchar_t*)(serialize_recordBuf.getBuffer()), serialize_recordBuf.getBufferSize()).c_str());
  if ( fileDispatcher.writeNbytesToFile(ioPage, reinterpret_cast<uchar_t*>(serialize_recordBuf.getBuffer()), serialize_recordBuf.getBufferSize()) < 0 ) {
    smsc_log_error(_logger, "SimpleFileDispatcher<V>::marshal::: can't write to file");
    return DataStorage_FileDispatcher<V>::FATAL;
  } else
    return DataStorage_FileDispatcher<V>::OPERATION_OK;
}

template <typename V>
typename DataStorage_FileDispatcher<V>::operation_status_t
SimpleFileDispatcher<V>::StorableRecord::unmarshal(IOPage& ioPage, SimpleFileDispatcher<V>& fileDispatcher)
{
  uchar_t markerBuf[RECORD_MARKER_SZ];
  if ( fileDispatcher.readNbytesFromFile(ioPage, markerBuf, sizeof(markerBuf)) < 0 ||
       memcmp(markerBuf, record_marker_constant, sizeof(record_marker_constant)) ) {
    smsc_log_error(_logger, "SimpleFileDispatcher<V>::unmarshal::: wrong value of begin record marker");
    return DataStorage_FileDispatcher<V>::FILE_DAMAGED;
  }
  uchar_t recordSizeBuf[RECORD_LENGTH_SZ];

  if ( fileDispatcher.readNbytesFromFile(ioPage, recordSizeBuf, sizeof(recordSizeBuf)) < 0 ) {
    smsc_log_error(_logger, "SimpleFileDispatcher<V>::unmarshal::: read recordSizeBuf failed");
    return DataStorage_FileDispatcher<V>::FILE_DAMAGED;
  }

  smsc::util::SerializationBuffer serialize_recordSizeBuf;
  serialize_recordSizeBuf.setExternalBuffer(recordSizeBuf, sizeof(recordSizeBuf));
  _recordSz = serialize_recordSizeBuf.ReadNetInt16();

  if ( _recordSz == 0 ) {
    smsc_log_error(_logger, "SimpleFileDispatcher<V>::unmarshal::: wrong _recordSz value=0",_recordSz);
    return DataStorage_FileDispatcher<V>::FILE_DAMAGED;
  }

  _crc=smsc::util::crc32(0, &_recordSz, sizeof(_recordSz));

  if ( fileDispatcher.readNbytesFromFile(ioPage, &_flg, sizeof(_flg)) < 0 ) {
    smsc_log_error(_logger, "SimpleFileDispatcher<V>::unmarshal::: can't read flg");
    return DataStorage_FileDispatcher<V>::FILE_DAMAGED;
  }
  _crc=smsc::util::crc32(_crc, &_flg, sizeof(_flg));

  smsc::util::SerializationBuffer serialize_recordBuf(_recordSz);

  if ( fileDispatcher.readNbytesFromFile(ioPage,reinterpret_cast<uchar_t*>(serialize_recordBuf.getBuffer()), serialize_recordBuf.getBufferSize()) < 0 ) {
    smsc_log_error(_logger, "SimpleFileDispatcher<V>::unmarshal::: can't read serialize buf");
    return DataStorage_FileDispatcher<V>::FILE_DAMAGED;
  }
  _crc=smsc::util::crc32(_crc, serialize_recordBuf.getBuffer(), serialize_recordBuf.getBufferSize());

  uchar_t crcBuf[RECORD_CRC_SZ];
  if ( fileDispatcher.readNbytesFromFile(ioPage, crcBuf, sizeof(crcBuf)) < 0 ) {
    smsc_log_error(_logger, "SimpleFileDispatcher<V>::unmarshal::: can't read crc");
    return DataStorage_FileDispatcher<V>::FILE_DAMAGED;
  }
  smsc::util::SerializationBuffer serialize_crcBuf;
  serialize_crcBuf.setExternalBuffer(crcBuf, RECORD_CRC_SZ);

  if ( serialize_crcBuf.ReadNetInt32() != _crc ) {
    smsc_log_error(_logger, "SimpleFileDispatcher<V>::unmarshal::: crc doesn't match, calced crc=%08X",_crc );
    smsc_log_error(_logger, "SimpleFileDispatcher<V>::unmarshal::: wrong buf=%s %02X %s %s ", hexdmp(recordSizeBuf, sizeof(recordSizeBuf)).c_str(), _flg, hexdmp((uchar_t*)(serialize_recordBuf.getBuffer()), serialize_recordBuf.getBufferSize()).c_str(), hexdmp(crcBuf, sizeof(crcBuf)).c_str());
    return DataStorage_FileDispatcher<V>::FILE_DAMAGED;
  }
  if ( _flg & RECORD_DELETED_FLAG )
    return DataStorage_FileDispatcher<V>::RECORD_DELETED;
  else {
    //    delete _readingAppData;
    //    _readingAppData = new V(serialize_recordBuf);

    if ( _readingAppData ) 
      _readingAppData = new (_readingAppData) V(serialize_recordBuf);
    else
      _readingAppData = new V(serialize_recordBuf);

    return DataStorage_FileDispatcher<V>::OPERATION_OK;
  }
}

#endif // templates only
