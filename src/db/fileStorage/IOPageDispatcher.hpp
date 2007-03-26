#ifndef __DBENTITYSTORAGE_IOPAGEDISPATCHER_HPP__
# define __DBENTITYSTORAGE_IOPAGEDISPATCHER_HPP__ 1

# include <sys/types.h>
# include <inttypes.h>
# include <map>
# include <core/synchronization/Mutex.hpp>

class IOPageDispatcher;

class IOPage_impl {
public:
  IOPage_impl(size_t pageNum, uint8_t* pageDataPtr, size_t actualDataSize, IOPageDispatcher* dispatcher);
  ~IOPage_impl();
  ssize_t setPosition(off_t* inPageOffset, off_t byteOffset, int whence);
  ssize_t read(off_t* inPageOffset, uint8_t *buf, size_t bufSz);
  ssize_t readOrderReverse(off_t* inPageOffset, uint8_t *buf, size_t bufSz);
  ssize_t write(off_t* inPageOffset, const uint8_t *buf, size_t bufSz);
  int commitMemory();

  void markUnusedData();

  off_t getAbsolutPosition(off_t inPageOffset) const;
  size_t getFreeSize() const;

  int flush(int fd, off_t recordFilePosition);

  int getPageNum() const;
  enum {PAGE_SIZE=4096} page_size_t;
  uint8_t* returnAllocatedMemory();
private:
  const size_t _pageNum;
  uint8_t* _pageDataPtr;
  size_t _actualDataSize;
  IOPageDispatcher* _dispatcher;

  typedef std::map<off_t,size_t> uncommited_memory_chunk_t;
  uncommited_memory_chunk_t _uncommited_memory_chunks;

#ifdef NEED_IOPAGE_DISPATCHER_LOCK
  mutable pthread_rwlock_t _page_IO_RwLock;
#endif
};

class IOPage {
public:
  IOPage(IOPage_impl* page, off_t inPageOffset) : _page(page), _inPageOffset(inPageOffset) {}

  ssize_t setPosition(off_t rid, int whence) {
    return _page->setPosition(&_inPageOffset, rid, whence);
  }
  ssize_t read(uint8_t *buf, size_t bufSz) {
    return _page->read(&_inPageOffset, buf, bufSz);
  }
  ssize_t readOrderReverse(uint8_t *buf, size_t bufSz) {
    ssize_t reverseCnt=0;
    if ( _inPageOffset > 0 )
      reverseCnt = _page->readOrderReverse(&_inPageOffset, buf, bufSz);
    return reverseCnt;
  }
  ssize_t write(const uint8_t *buf, size_t bufSz) {
    return _page->write(&_inPageOffset, buf, bufSz);
  }
  int commitMemory() {
    return _page->commitMemory();
  }
  size_t getFreeSize() const {
    return _page->getFreeSize();
  }
  void markUnusedData() {
    _page->markUnusedData();
  }
  off_t getAbsolutPosition() const {
    return _page->getAbsolutPosition(_inPageOffset);
  }
  int getPageNum() const {
    return _page->getPageNum();
  }
  size_t getRemainderSizeToEndPage() const {
    return IOPage_impl::PAGE_SIZE - _inPageOffset;
  }
  size_t getPageSize() const {
    return IOPage_impl::PAGE_SIZE;
  }
private:
  IOPage_impl* _page;
  off_t _inPageOffset;
};

class IOPageDispatcher {
public:
  IOPageDispatcher(int fd);
  ~IOPageDispatcher();
  virtual IOPage getFirstIOPage();
  virtual IOPage getNextIOPage(const IOPage& currentPage);
  virtual IOPage getPreviousIOPage(const IOPage& currentPage);
  virtual IOPage getLastIOPage();

  virtual IOPage createNewIOPage() = 0;
  virtual IOPage getIOPage(off_t rid) = 0;
  virtual bool haveNextPage(const IOPage& currentPage) = 0;

  virtual void markPageAsDitry(IOPage_impl* dirtyPage);
  virtual int commitDirtyPage(IOPage_impl* dirtyPage);
protected:
  int _fd;
  const off_t _firstPageOffset, _startByteOffset;
  off_t _lastPageOffset;
  typedef std::map<int, IOPage_impl*> registredPagesMap_t;
  registredPagesMap_t _registredPages;

#ifdef NEED_IOPAGE_DISPATCHER_LOCK
  smsc::core::synchronization::Mutex _lastPageLock, _registredPagesLock;
#endif

  IOPage_impl* loadPageData(int pageNum, IOPage_impl* pagePtr=0);
private:
  IOPageDispatcher(const IOPageDispatcher& rhs);
  IOPageDispatcher& operator=(const IOPageDispatcher& rhs);
};

#endif
