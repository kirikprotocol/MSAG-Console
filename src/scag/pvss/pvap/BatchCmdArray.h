#ifndef _SCAG_PVSS_PVAP_BATCHCMDARRAY_H
#define _SCAG_PVSS_PVAP_BATCHCMDARRAY_H

namespace scag {
namespace pvss {
namespace pvap {

class BC_CMD;

class BatchCmdArray
{
public:
    BatchCmdArray();
    BatchCmdArray( const BatchCmdArray& a );
    std::string toString() const;
};

} // namespace pvap
} // namespace pvss
} // namespace scag

#endif /* ! _SCAG_PVSS_PVAP_BATCHCMDARRAY_H */
