/*
  $Id$
*/


#if !defined __Cpp_Header__smeman_smeman_h__
#define __Cpp_Header__smeman_smeman_h__
#include <inttypes.h>
#include <vector>
#include <list>

#include "util/debug.h"
#include "smetypes.h"
#include "smeproxy.h"
#include "smeinfo.h"
#include "smeadmin.h"
#include "smetable.h"
#include "smeiter.h"
#include "smereg.h"
#include "smedispatch.h"
#include "dispatch.h"
#include "core/synchronization/Mutex.hpp"

namespace smsc {
namespace smeman {
  
struct SmeRecord
{
  SmeInfo info;
  SmeProxy* proxy;
  bool deleted;
  SmeIndex idx;
};
  
using core::synchronization::Mutex; 
typedef std::vector<SmeRecord> Records;

class SmeManager : 
  public SmeAdministrator,
  public SmeTable,
  public SmeRegistrar,
  public Dispatch
{
    
  mutable Mutex lock;
  SmeProxyDispatcher dispatcher;
  Records records;
  SmeIndex internalLookup(const SmeSystemId& systemId) const;
public: 
  //....
  // SmeAdministrator implementation
  virtual void addSme(const SmeInfo& info);
  virtual void deleteSme(const SmeSystemId& systemId);
//  virtual void store();
  virtual SmeIterator* iterator(); // client must destroy returned object
  virtual void disableSme(const SmeSystemId& systemId);
  virtual void enableSme(const SmeSystemId& systemId);

  // SmeTable implementation
  virtual SmeIndex lookup(const SmeSystemId& systemId) const;
  virtual SmeProxy* getSmeProxy(SmeIndex index) const;
  virtual SmeInfo getSmeInfo(SmeIndex index) const;

/*
  // SmeIterator implementation
  virtual bool next();
  virtual SmeProxy* getSmeProxy() const;
  virtual SmeInfo  getSmeInfo() const;
  virtual SmeIndex getSmeIndex() const; // ?????
*/
  // SmeRegistrar implementation
  virtual void registerSmeProxy(const SmeSystemId& systemId, SmeProxy* smeProxy);
  virtual void unregisterSmeProxy(const SmeSystemId& systemId);

  // SmeDispatcher implementation
  virtual SmeProxy* selectSmeProxy(unsigned long timeout=0,int* idx=0);
};

}; // namespace smeman
}; // namespace smsc
#endif



