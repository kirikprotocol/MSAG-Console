#ifndef __SMSC_BDB_AGENT_SINGLESHAREDQUEUE_HPP__
# define __SMSC_BDB_AGENT_SINGLESHAREDQUEUE_HPP__ 1 

# include <util/Singleton.hpp>
# include <util/comm_comp/ObjQueue.hpp>

namespace smsc {
namespace util {
namespace comm_comp {

/*
** Queue of objects type T with singleton semantics.
*/
template <class T>
class SingleSharedQueue : public smsc::util::Singleton<SingleSharedQueue<T> >,
                          public ObjQueue<T> {};

}}}

#endif
