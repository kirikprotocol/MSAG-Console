#ifndef _SCAG_PVSS_PVAP_BatchCmd_H
#define _SCAG_PVSS_PVAP_BatchCmd_H

namespace scag {
namespace pvss {
namespace pvap {

class BatchCmd
{
public:
    virtual ~BatchCmd() {}

    /// NOTE: due to a severe limitation of protogen this method returns
    /// not protocol's tag, but some arbitrary unique integer value.
    /// You should compare this id with TypeId< Type >::getId().
    virtual int getId() const throw () = 0;

    virtual std::string toString() const = 0;
};

} // namespace pvap
} // namespace pvss
} // namespace scag

#endif
