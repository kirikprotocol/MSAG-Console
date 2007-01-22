#ifndef __DBENTITYSTORAGE_SEQUNCENUMBER_HPP__
# define __DBENTITYSTORAGE_SEQUNCENUMBER_HPP__ 1

# include <sys/types.h>
# include <sys/stat.h>
# include <fcntl.h>
# include <sys/mman.h>
# include <unistd.h>

# include <string>
# include <util/Singleton.hpp>
# include <iostream>
# include <util/Exception.hpp>
# include <core/synchronization/Mutex.hpp>

class SequenceNumber : public smsc::util::Singleton<SequenceNumber>
{
public:
  SequenceNumber() : _fd(-1), _seqNum(NULL) {}
  ~SequenceNumber() { ::munmap(reinterpret_cast<char*>(_seqNum), sizeof(seq_num_t)); close(_fd); }

  void initialize(const std::string& seqFileName)
  {
    if ( _fd < 0 ) {
      if ( (_fd = ::open(seqFileName.c_str(), O_RDWR|O_CREAT|O_EXCL, S_IRUSR | S_IWUSR)) < 0 ) {
        if ( errno != EEXIST )
          throw smsc::util::Exception("SequenceNumber::initialize::: can't create file for storing of sequence number");
        else {
          if ( (_fd = ::open(seqFileName.c_str(), O_RDWR, 0)) < 0 )
            throw smsc::util::Exception("SequenceNumber::initialize::: can't open file for storing of sequence number");
        }
      } else {
        seq_num_t zeroValue=0;
        if ( write(_fd, reinterpret_cast<uint8_t*>(&zeroValue), sizeof(zeroValue)) != sizeof(zeroValue))
          throw smsc::util::Exception("SequenceNumber::initialize::: can't assgin initial value to sequence number");
      }

      _seqNum = reinterpret_cast<seq_num_t*>(::mmap(0, sizeof(seq_num_t), PROT_READ|PROT_WRITE, MAP_SHARED, _fd, 0));
      if ( _seqNum == MAP_FAILED )
        throw smsc::util::Exception("SequenceNumber::initialize::: can't map file data to memory");
    }
  }

  uint64_t getNextSequenceNumber()
  {
    smsc::core::synchronization::MutexGuard lockGuard(_seqAccessLock);
    return ++*_seqNum;
  }

private:
  typedef uint64_t seq_num_t;
  int _fd;
  seq_num_t* _seqNum;
  smsc::core::synchronization::Mutex _seqAccessLock;
};



#endif
