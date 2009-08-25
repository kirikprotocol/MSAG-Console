#ifndef __DBENTITYSTORAGE_DISKDATASTORAGE_HPP__
# define __DBENTITYSTORAGE_DISKDATASTORAGE_HPP__ 1

# include <sys/types.h>
# include <netinet/in.h>
# include <inttypes.h>
# include <string>
# include <string.h>
# include <util/BufferSerialization.hpp>

#ifdef __GNUC__
typedef unsigned char uchar_t;
#endif


extern std::string hexdmp( uchar_t* buf, uint32_t bufsz );


struct DataStorage_FileDispatcher_Status {
  typedef enum { OPERATION_OK=0, STORAGE_CLOSED, NO_SUCH_FILE, STORAGE_ALREADY_EXISTS,
                 NO_RECORD_FOUND=100, RECORD_DELETED, REPLACEMENT_NOT_ALLOWED,
                 FILE_DAMAGED=200, INVALID_FILE_VERSION,
                 FATAL=255 } operation_status_t;
};

/*
** Класс описывает интерфейс абстракции диспетчера файлов хранилища данных.
** Классы, реализующие данный интерфейс, ответственны за хранение записей в 
** некотором источнике данных и предоставление/модификацию этих данных
** по запросу. Методы класса оперируют понятием запись. Запись однозначно
** определяется идентификатором записи (record id, или rid ). Класс
** параметризуется типом V, представляющим сохраняемую запись. Интерфейс,
** представленный типом V, обязан предоставить метод для сохранения
** записи V::marshal(), конструктор, принимающий SerializationBuffer и методы
** для получения размера записи V::getSize() и для вычисления crc
** записи V::calcCrc().
*/
template<typename V>
  class DataStorage_FileDispatcher : public DataStorage_FileDispatcher_Status {
public:
  typedef off_t rid_t;

  // Конструктор создает объект, соответствующий хранилищу данных с именем файла,
  // переданным в аргументе.
  explicit DataStorage_FileDispatcher(const std::string& fileName);
  virtual ~DataStorage_FileDispatcher() {}
  // Открыть хранилище данных.
  virtual operation_status_t open() = 0;
  // Создать хранилище данных.
  virtual operation_status_t create() = 0;
  // Закрыть хранилище
  virtual operation_status_t close() = 0;
  // Уничтожить хранилище
  virtual operation_status_t drop() = 0;

  // Извлечь запись из хранилища по значению record id 
  virtual operation_status_t extractRecord(V* record, const rid_t& rid) = 0;
  // Извлечь первую запись из хранилища. Возвращает NO_RECORD_FOUND, если
  // в хранилище нет записей.
  virtual operation_status_t extractFirstRecord(V* record, rid_t* rid, rid_t* nextRid) = 0;
  // Извлечь следующую запись из хранилища. Возвращает NO_RECORD_FOUND, если
  // просмотрены все записи.
  virtual operation_status_t extractNextRecord(V* record, rid_t* rid, rid_t* nextRid) = 0;
  // Заменить запись в хранилище для указанного record id 
  virtual operation_status_t replaceRecord(const V& record, const rid_t& rid) = 0;
  // Добавить новую запись в хранилище. Для успешно созданной записи
  // возвращается record id
  virtual operation_status_t addRecord(const V& record, rid_t* rid) = 0;
  // Удалить запись из хранилища, соответствующую указанному record id
  virtual operation_status_t deleteRecord(const rid_t& rid) = 0;

  // Сбросить модифицированные данные на диск
  virtual void flush() = 0;

protected:
  std::string _storageFileName;

private:
  DataStorage_FileDispatcher(const DataStorage_FileDispatcher& rhs);
  DataStorage_FileDispatcher& operator=(const DataStorage_FileDispatcher& rhs);

};

static const size_t STORAGE_END_MARKER_SZ = 4;

static const size_t RECORD_MARKER_SZ=2, 
  RECORD_LENGTH_SZ=2,
  RECORD_FLG_SZ=1,
  RECORD_CRC_SZ=4,
  RECORD_OVERHEAD_SZ = RECORD_MARKER_SZ + RECORD_LENGTH_SZ + RECORD_FLG_SZ + RECORD_CRC_SZ;

static const size_t STORAGE_HEADER_SZ = 4 + 4; // версия (4байта) и reserved(4байта).

# include "IOPageDispatcher.hpp"
# include <core/synchronization/Mutex.hpp>

# include <logger/Logger.h>

template<typename V>
class SimpleFileDispatcher : public DataStorage_FileDispatcher<V> {
public:
  explicit SimpleFileDispatcher(const std::string& fileName) : DataStorage_FileDispatcher<V>(fileName), _fd(-1), _ioPageDispatcher(0),_logger(smsc::logger::Logger::getInstance("dbstrg")) {}
  virtual ~SimpleFileDispatcher();

  typedef typename DataStorage_FileDispatcher<V>::rid_t rid_t;
  typedef typename DataStorage_FileDispatcher<V>::operation_status_t operation_status_t;

  virtual operation_status_t open();

  virtual operation_status_t create();

  virtual operation_status_t close();

  virtual operation_status_t drop();

  virtual operation_status_t extractRecord(V* record, const rid_t& rid);
  virtual operation_status_t extractFirstRecord(V* record, rid_t* rid, rid_t* nextRid);
  virtual operation_status_t extractNextRecord(V* record, rid_t* rid, rid_t* nextRid);
  virtual operation_status_t replaceRecord(const V& record, const rid_t& rid);
  virtual operation_status_t addRecord(const V& record, rid_t* rid);

  virtual operation_status_t deleteRecord(const rid_t& rid);

  virtual void flush() {}

private:
  typedef enum { STORAGE_VERSION = 0x00000100 } version_magic_t;
  static uchar_t end_storage_marker_constant[STORAGE_END_MARKER_SZ];

  typedef enum {RECORD_DELETED_FLAG = 0x01} record_flags_t;
  static uchar_t record_marker_constant[RECORD_MARKER_SZ];

  /*
  ** Заголовок: версия (4байта), reserved(4байта).
  ** Записи следуют последовательно одна за другой.
  ** Формат записи: маркер начала записи(2 байта), размер прикладных данных 
  ** записи (2 байта), поле флагов(1 байт), прикладные данные,
  ** crc по данным записи (размер,флаг,данные) 4 байта.
  ** Маркеры начала записи должен иметь значение 0x00 0x00. Запись с нулевым
  ** размером не допускается
  ** Список записей завершается терминирующим маркером - последовательность из 4 байт,
  ** имеющая значение 0xAA 0x11 0xEE 0x33
  */

  class StorableRecord {
  public:
    StorableRecord() : _recordSz(0), _flg(0), _writingAppData(NULL), _readingAppData(NULL), _crc(0),_logger(smsc::logger::Logger::getInstance("dbstrg")) {}
    StorableRecord(const V* appData, uint8_t flg) : _recordSz(appData->getSize()),_writingAppData(appData), _readingAppData(NULL), _flg(flg), _logger(smsc::logger::Logger::getInstance("dbstrg")) {}
    ~StorableRecord() { delete _readingAppData; }
    operation_status_t marshal(IOPage& ioPage, SimpleFileDispatcher<V>& fileDispatcher);
    operation_status_t unmarshal(IOPage& ioPage, SimpleFileDispatcher<V>& fileDispatcher);
    uint8_t getFlg() const { return _flg; }
    void setFlg(uint8_t flg) { _flg = flg; }
    V* getReadRecord() const { return _readingAppData; }
  private:
    uint16_t _recordSz;
    uint8_t _flg;
    const V* _writingAppData;
    V* _readingAppData;
    uint32_t _crc;
    smsc::logger::Logger* _logger;
  };
  int _fd;

  operation_status_t
  checkEndStorageMarker(const IOPage& ioPage);

  int readNbytesFromFile(IOPage& ioPage, uchar_t* buf, size_t bufSz);
  int writeNbytesToFile(IOPage& ioPage, uchar_t* buf, size_t bufSz);

  IOPageDispatcher* _ioPageDispatcher;
  smsc::core::synchronization::Mutex _addRecordLock;

  smsc::logger::Logger* _logger;
};

#ifdef TEMPINST
#ifndef __DBENTITYSTORAGE_DISKDATASTORAGE_CPP__
#include "DiskDataStorage.cpp"
#endif
#endif

#endif
