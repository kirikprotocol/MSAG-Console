#ifndef _SCAG_PVSS_PVAP_BC_CMD_H
#define _SCAG_PVSS_PVAP_BC_CMD_H

namespace scag {
namespace pvss {
namespace pvap {

class BC_CMD
{
public:
    virtual ~BC_CMD() {}

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
