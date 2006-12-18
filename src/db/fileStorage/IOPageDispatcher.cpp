#include "IOPageDispatcher.hpp"
#include <unistd.h>
#include <string>
#include <stdexcept>
#include <string.h>
#include <utility>

#include <pthread.h>

class ReadLockGuard {
public:
  ReadLockGuard(pthread_rwlock_t& lock) : _lock(lock) {
    pthread_rwlock_rdlock(&_lock);
  }
  ~ReadLockGuard() {
    pthread_rwlock_unlock(&_lock);
  }
private:
  pthread_rwlock_t& _lock;
};

class WriteLockGuard {
public:
  WriteLockGuard(pthread_rwlock_t& lock) : _lock(lock) {
    pthread_rwlock_wrlock(&_lock);
  }
  ~WriteLockGuard() {
    pthread_rwlock_unlock(&_lock);
  }
private:
  pthread_rwlock_t& _lock;
};

IOPageDispatcher::IOPageDispatcher(int fd) : _fd(fd), _firstPageOffset(0), _startByteOffset(::lseek(_fd, 0, SEEK_CUR)), _lastPageOffset(0) {
  if ( _startByteOffset < 0 )
    throw std::runtime_error("IOPageDispatcher::IOPageDispatcher::: lseek failed");
}

IOPage
IOPageDispatcher::getIOPage(off_t byteOffset)
{
  int pageNum = byteOffset / IOPage_impl::PAGE_SIZE;

  smsc::core::synchronization::MutexGuard mutexGuard(_registredPagesLock);
  registredPagesMap_t::iterator regPageIter = _registredPages.find(pageNum);
  if ( regPageIter == _registredPages.end() ) {
    uint8_t* pageDataPtr = new uint8_t[IOPage_impl::PAGE_SIZE];
    ssize_t sz;
    {
      if ( ::lseek(_fd, _startByteOffset + pageNum * IOPage_impl::PAGE_SIZE, SEEK_SET) < 0 ||
           ((sz=::read(_fd, pageDataPtr, IOPage_impl::PAGE_SIZE)) < 0) )
        return IOPage(NULL,0);
    }
    ::memset(pageDataPtr+sz, 0xFF, IOPage_impl::PAGE_SIZE-sz);

    std::pair<registredPagesMap_t::iterator, bool> insResult = _registredPages.insert(std::make_pair(pageNum, new IOPage_impl(pageNum, pageDataPtr, sz, this)));
    if (insResult.second)
      regPageIter = insResult.first;
    else return IOPage(NULL,0);
  }
  return IOPage(regPageIter->second, byteOffset % IOPage_impl::PAGE_SIZE);
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
  smsc::core::synchronization::MutexGuard mutexGuard(_lastPageLock);
  return getIOPage(_lastPageOffset);
}

IOPage
IOPageDispatcher::createNewIOPage()
{
  smsc::core::synchronization::MutexGuard mutexGuard(_lastPageLock);
  _lastPageOffset += IOPage_impl::PAGE_SIZE;
  return getIOPage(_lastPageOffset);
}

void
IOPageDispatcher::markPageAsDitry(IOPage_impl* dirtyPage)
{
  
}

int
IOPageDispatcher::commitDirtyPage(IOPage_impl* dirtyPage)
{
  smsc::core::synchronization::MutexGuard mutexGuard(_registredPagesLock);
  off_t recordFilePosition = ::lseek(_fd, _startByteOffset + dirtyPage->getPageNum() * IOPage_impl::PAGE_SIZE, SEEK_SET);

  if ( recordFilePosition == (off_t)-1 )
    return -1;

  return dirtyPage->flush(_fd, recordFilePosition);
}

IOPage_impl::IOPage_impl(size_t pageNum, uint8_t* pageDataPtr, size_t actualDataSize, IOPageDispatcher* dispatcher) : _pageNum(pageNum), _pageDataPtr(pageDataPtr), _actualDataSize(actualDataSize), _dispatcher(dispatcher) {
  if ( pthread_rwlock_init(&_page_IO_RwLock, NULL) )
    throw std::runtime_error("IOPage_impl::IOPage_impl::: can't initialize rwlock");
}

IOPage_impl::~IOPage_impl()
{
  pthread_rwlock_destroy(&_page_IO_RwLock);
}

ssize_t
IOPage_impl::setPosition(off_t* inPageOffset, off_t byteOffset, int whence)
{
  if ( whence == SEEK_SET ) {
    *inPageOffset = byteOffset % PAGE_SIZE;
  } else { // corresponds to SEEK_CUR or SEEK_END
    ReadLockGuard rwlockGuard(_page_IO_RwLock);
    if ( whence == SEEK_END ) {
      *inPageOffset = _actualDataSize + byteOffset;
    } else { // SEEK_CUR
      *inPageOffset += byteOffset;
    }

    if ( *inPageOffset >= PAGE_SIZE )
      return -1;
  }

  return _pageNum*PAGE_SIZE + *inPageOffset;
}

ssize_t
IOPage_impl::read(off_t* inPageOffset, uint8_t *buf, size_t bufSz)
{
  ReadLockGuard rwlockGuard(_page_IO_RwLock);

  size_t readeableSize = std::min(ulong_t(PAGE_SIZE - *inPageOffset), bufSz);

  ::memcpy(buf, _pageDataPtr+*inPageOffset, readeableSize);
  *inPageOffset += bufSz;

  return readeableSize;
}

ssize_t
IOPage_impl::readOrderReverse(off_t* inPageOffset, uint8_t *buf, size_t bufSz)
{
  ReadLockGuard rwlockGuard(_page_IO_RwLock);
  int i=bufSz;
  while (i>0 && *inPageOffset>=0) {
    buf[i-1] = _pageDataPtr[*inPageOffset];
    --i; --*inPageOffset;
  }

  return bufSz - i;
}

ssize_t
IOPage_impl::write(off_t* inPageOffset, const uint8_t *buf, size_t bufSz)
{
  size_t writeableSize = std::min(ulong_t(PAGE_SIZE - *inPageOffset), bufSz);

  WriteLockGuard rwlockGuard(_page_IO_RwLock);
  ::memcpy(_pageDataPtr+*inPageOffset, buf, writeableSize);

  _uncommited_memory_chunks.insert(std::make_pair(*inPageOffset, writeableSize));

  *inPageOffset += writeableSize;

  if ( _actualDataSize < *inPageOffset )
    _actualDataSize = *inPageOffset;

  return writeableSize;
}

int
IOPage_impl::commitMemory()
{
  WriteLockGuard rwlockGuard(_page_IO_RwLock);
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
  ReadLockGuard rwlockGuard(_page_IO_RwLock);
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
