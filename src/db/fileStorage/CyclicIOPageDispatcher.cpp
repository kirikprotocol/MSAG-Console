#include "CyclicIOPageDispatcher.hpp"
#include <string>
#include <stdexcept>
#include <assert.h>

#ifdef __GNUC__
typedef unsigned char uchar_t;
#endif

// for debug 
extern std::string
hexdmp(uchar_t* buf, uint32_t bufSz);
// end for debug

CyclicIOPageDispatcher::CyclicIOPageDispatcher(int fd) : IOPageDispatcher(fd),
                                                         _minPageNumInCache(0), _maxPageNumInCache(0)
{
  if ( _startByteOffset < 0 )
    throw std::runtime_error("CyclicIOPageDispatcher::CyclicIOPageDispatcher::: lseek failed");
  _registredPages.insert(std::make_pair(0,loadPageData(0)));
}

bool
CyclicIOPageDispatcher::haveNextPage(const IOPage& currentPage)
{
#ifdef NEED_IOPAGE_DISPATCHER_LOCK
  smsc::core::synchronization::MutexGuard lastPageMutexGuard(_lastPageLock);
#endif
  if ( currentPage.getPageNum() == _lastPageOffset/IOPage_impl::PAGE_SIZE ) return false;
  else return true;
}

IOPage
CyclicIOPageDispatcher::getIOPage(off_t byteOffset)
{
  int pageNum = byteOffset / IOPage_impl::PAGE_SIZE;

#ifdef NEED_IOPAGE_DISPATCHER_LOCK
  smsc::core::synchronization::MutexGuard mutexGuard(_registredPagesLock);
#endif
  registredPagesMap_t::iterator regPageIter = _registredPages.find(pageNum);

  IOPage_impl* ioPage;
  if ( regPageIter != _registredPages.end() ) {
    ioPage = regPageIter->second;
  } else {
    std::pair<registredPagesMap_t::iterator, bool> insResult;
    if ( _registredPages.size() < MAX_CACHED_PAGES_NUM ) {
      insResult = _registredPages.insert(std::make_pair(pageNum,loadPageData(pageNum)));
      assert(insResult.second);
      ioPage = insResult.first->second;
      if ( pageNum > _maxPageNumInCache )
        _maxPageNumInCache = pageNum;
      else if ( pageNum < _minPageNumInCache )
        _minPageNumInCache = pageNum;
    } else {
      registredPagesMap_t::iterator iter;
      IOPage_impl* allocatedMemForPage;
      if ( pageNum > _maxPageNumInCache ) { // добавляется страница в конец циклического кеша
        // удаляем первый элемент из циклического кеша
        iter = _registredPages.begin();
        registredPagesMap_t::iterator iterForNextElement = iter;
        allocatedMemForPage = iter->second;
        ++iterForNextElement; _minPageNumInCache = iterForNextElement->first;
        _maxPageNumInCache = pageNum;
      } else if ( pageNum < _minPageNumInCache ) { // добавляется страница в начало циклического кеша
        // удаляем последний элемент из цикличекого кеша
        iter = _registredPages.end(); --iter;
        allocatedMemForPage = iter->second;
        registredPagesMap_t::iterator iterForPrevElement = iter;
        _minPageNumInCache = pageNum; --iterForPrevElement;
        _maxPageNumInCache = iterForPrevElement->first;
      } else { // не краевой слуай
        iter = _registredPages.lower_bound(pageNum);
        registredPagesMap_t::iterator iterForNextElement = iter;
        if ( ++iterForNextElement == _registredPages.end() )
          _maxPageNumInCache = pageNum;
        allocatedMemForPage = iter->second;
      }
      _registredPages.erase(iter);
      std::pair<registredPagesMap_t::iterator, bool> insResult;
      insResult = _registredPages.insert(std::make_pair(pageNum,loadPageData(pageNum,allocatedMemForPage)));

      assert(insResult.second);
      ioPage = insResult.first->second;
    }
  }
  return IOPage(ioPage, byteOffset % IOPage_impl::PAGE_SIZE);
}

IOPage
CyclicIOPageDispatcher::createNewIOPage()
{
#ifdef NEED_IOPAGE_DISPATCHER_LOCK
  smsc::core::synchronization::MutexGuard lastPageMutexGuard(_lastPageLock);
#endif

  _lastPageOffset += IOPage_impl::PAGE_SIZE;
  return getIOPage(_lastPageOffset);
}
