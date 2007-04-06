#include "IOPageDispatcher.hpp"
#include <unistd.h>
#include <string>
#include <stdexcept>
#include <string.h>
#include <utility>
#include <algorithm>

#include <pthread.h>

#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "RWLockGuard.hpp"
#include <assert.h>

IOPageDispatcher::IOPageDispatcher(int fd) : _fd(fd), _firstPageOffset(0), _startByteOffset(::lseek(_fd, 0, SEEK_CUR)), _lastPageOffset(::lseek(_fd, 0, SEEK_END)) {
  assert( _startByteOffset >= 0 && _lastPageOffset >= 0 );
  _lastPageOffset = ((off_t)(_lastPageOffset / IOPage_impl::PAGE_SIZE)) * IOPage_impl::PAGE_SIZE;
  ::lseek(_fd, _startByteOffset, SEEK_SET);
}

IOPageDispatcher::~IOPageDispatcher()
{
  while (!_registredPages.empty()) {
    IOPage_impl* ioPage = _registredPages.begin()->second;
    delete ioPage;
    _registredPages.erase(_registredPages.begin());
  }
}

IOPage
IOPageDispatcher::getFirstIOPage()
{
  return getIOPage(_firstPageOffset);
}

IOPage
IOPageDispatcher::getNextIOPage(const IOPage& currentPage)
{
  return getIOPage((currentPage.getPageNum()+1)*IOPage_impl::PAGE_SIZE);
}

IOPage
IOPageDispatcher::getPreviousIOPage(const IOPage& currentPage)
{
  if ( currentPage.getPageNum()-1 < 0 )
    throw std::runtime_error("IOPageDispatcher::getPreviousIOPage::: previous page doesn't exist");
  return getIOPage((currentPage.getPageNum()-1)*IOPage_impl::PAGE_SIZE);
}

IOPage
IOPageDispatcher::getLastIOPage()
{
#ifdef NEED_IOPAGE_DISPATCHER_LOCK
  smsc::core::synchronization::MutexGuard mutexGuard(_lastPageLock);
#endif
  return getIOPage(_lastPageOffset);
}

IOPage_impl*
IOPageDispatcher::loadPageData(int pageNum, IOPage_impl* pagePtr)
{
  uint8_t *pageDataPtr;
  if ( ! pagePtr )
    pageDataPtr = new uint8_t[IOPage_impl::PAGE_SIZE];
  else {
    pageDataPtr = pagePtr->returnAllocatedMemory();
    pagePtr->~IOPage_impl();
  }
  ssize_t sz;

  if ( ::lseek(_fd, _startByteOffset + pageNum * IOPage_impl::PAGE_SIZE, SEEK_SET) < 0 ||
       ((sz=::read(_fd, pageDataPtr, IOPage_impl::PAGE_SIZE)) < 0) ) {
    delete [] pageDataPtr;
    throw std::runtime_error("IOPageDispatcher::loadPageData::: can't extend file");
  }
  ::memset(pageDataPtr+sz, 0xFF, IOPage_impl::PAGE_SIZE-sz);

  if ( pagePtr )
    return new (pagePtr)IOPage_impl(pageNum, pageDataPtr, sz, this);
  else
    return new IOPage_impl(pageNum, pageDataPtr, sz, this);
}

void
IOPageDispatcher::markPageAsDitry(IOPage_impl* dirtyPage)
{
  
}

int
IOPageDispatcher::commitDirtyPage(IOPage_impl* dirtyPage)
{
#ifdef NEED_IOPAGE_DISPATCHER_LOCK
  smsc::core::synchronization::MutexGuard mutexGuard(_registredPagesLock);
#endif
  off_t recordFilePosition = ::lseek(_fd, _startByteOffset + dirtyPage->getPageNum() * IOPage_impl::PAGE_SIZE, SEEK_SET);

  if ( recordFilePosition == (off_t)-1 )
    return -1;

  return dirtyPage->flush(_fd, recordFilePosition);
}

IOPage_impl::IOPage_impl(size_t pageNum, uint8_t* pageDataPtr, size_t actualDataSize, IOPageDispatcher* dispatcher) : _pageNum(pageNum), _pageDataPtr(pageDataPtr), _actualDataSize(actualDataSize), _dispatcher(dispatcher) {
#ifdef NEED_IOPAGE_DISPATCHER_LOCK
  if ( pthread_rwlock_init(&_page_IO_RwLock, NULL) )
    throw std::runtime_error("IOPage_impl::IOPage_impl::: can't initialize rwlock");
#endif
}

IOPage_impl::~IOPage_impl()
{
#ifdef NEED_IOPAGE_DISPATCHER_LOCK
  pthread_rwlock_destroy(&_page_IO_RwLock);
#endif
  if ( _pageDataPtr )
    delete [] _pageDataPtr;
}

uint8_t*
IOPage_impl::returnAllocatedMemory()
{
  uint8_t* pageDataPtr = _pageDataPtr;
  _pageDataPtr = (uint8_t*)NULL;
  return pageDataPtr;
}

ssize_t
IOPage_impl::setPosition(off_t* inPageOffset, off_t byteOffset, int whence)
{
  if ( whence == SEEK_SET ) {
    *inPageOffset = byteOffset % PAGE_SIZE;
  } else { // corresponds to SEEK_CUR or SEEK_END
#ifdef NEED_IOPAGE_DISPATCHER_LOCK
    ReadLockGuard rwlockGuard(_page_IO_RwLock);
#endif
    off_t saveInPageOffset=*inPageOffset;
    if ( whence == SEEK_END ) {
      *inPageOffset = _actualDataSize + byteOffset;
    } else { // SEEK_CUR
      *inPageOffset += byteOffset;
    }

    if ( *inPageOffset > PAGE_SIZE || *inPageOffset < 0 ) {
      *inPageOffset = saveInPageOffset;
      return -1;
    }
  }

  return _pageNum*PAGE_SIZE + *inPageOffset;
}

ssize_t
IOPage_impl::read(off_t* inPageOffset, uint8_t *buf, size_t bufSz)
{
#ifdef NEED_IOPAGE_DISPATCHER_LOCK
  ReadLockGuard rwlockGuard(_page_IO_RwLock);
#endif
  size_t readeableSize = std::min(size_t(PAGE_SIZE - *inPageOffset), bufSz);
  if (readeableSize) {
    ::memcpy(buf, _pageDataPtr+*inPageOffset, readeableSize);
    *inPageOffset += readeableSize;
  }

  return readeableSize;
}

ssize_t
IOPage_impl::readOrderReverse(off_t* inPageOffset, uint8_t *buf, size_t bufSz)
{
#ifdef NEED_IOPAGE_DISPATCHER_LOCK
  ReadLockGuard rwlockGuard(_page_IO_RwLock);
#endif
  int i=bufSz;
  if ( !*inPageOffset )
    return 0;

  while (i>0 && *inPageOffset>0)
    buf[--i] = _pageDataPtr[--*inPageOffset];

  return bufSz - i;
}

ssize_t
IOPage_impl::write(off_t* inPageOffset, const uint8_t *buf, size_t bufSz)
{
  size_t writeableSize = std::min(size_t(PAGE_SIZE - *inPageOffset), bufSz);

  if (writeableSize) {
#ifdef NEED_IOPAGE_DISPATCHER_LOCK
    WriteLockGuard rwlockGuard(_page_IO_RwLock);
#endif
    ::memcpy(_pageDataPtr+*inPageOffset, buf, writeableSize);

    _uncommited_memory_chunks.insert(std::make_pair(*inPageOffset, writeableSize));

    *inPageOffset += writeableSize;

    if ( _actualDataSize < *inPageOffset )
      _actualDataSize = *inPageOffset;
  }

  return writeableSize;
}

int
IOPage_impl::commitMemory()
{
#ifdef NEED_IOPAGE_DISPATCHER_LOCK
  WriteLockGuard rwlockGuard(_page_IO_RwLock);
#endif
  return _dispatcher->commitDirtyPage(this);
}

int
IOPage_impl::flush(int fd, off_t recordFilePosition)
{
  while ( !_uncommited_memory_chunks.empty() ){
    uncommited_memory_chunk_t::iterator iter = _uncommited_memory_chunks.begin();
    if ( lseek(fd, recordFilePosition + iter->first, SEEK_SET) == (off_t)-1 )
      return -1;

    if ( ::write(fd, _pageDataPtr + iter->first, iter->second) != iter->second ) {
      _uncommited_memory_chunks.erase(_uncommited_memory_chunks.begin(),
                                      _uncommited_memory_chunks.end());
      return -1;
    }

    _uncommited_memory_chunks.erase(iter);
  }

  return 0;
}

size_t
IOPage_impl::getFreeSize() const
{
#ifdef NEED_IOPAGE_DISPATCHER_LOCK
  ReadLockGuard rwlockGuard(_page_IO_RwLock);
#endif
  return PAGE_SIZE - _actualDataSize;
}

void
IOPage_impl::markUnusedData()
{
}

off_t
IOPage_impl::getAbsolutPosition(off_t inPageOffset) const
{
  return _pageNum*PAGE_SIZE + inPageOffset;
}

int IOPage_impl::getPageNum() const
{
  return _pageNum;
}
