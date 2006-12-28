#ifndef __DBENTITYSTORAGE_CYCLICIOPAGEDISPATCHER_HPP__
# define __DBENTITYSTORAGE_CYCLICIOPAGEDISPATCHER_HPP__ 1

# include "IOPageDispatcher.hpp"

class CyclicIOPageDispatcher : public IOPageDispatcher {
public:
  CyclicIOPageDispatcher(int fd);
  virtual IOPage getIOPage(off_t rid);
  virtual IOPage createNewIOPage();
  virtual bool haveNextPage(const IOPage& currentPage);
private:
  enum {MAX_CACHED_PAGES_NUM = 32};
  int _minPageNumInCache, _maxPageNumInCache;
};

#endif
