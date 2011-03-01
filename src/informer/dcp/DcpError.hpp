#ifndef __EYELINE_INFORMER_DCP_DCPERROR_HPP__
#define __EYELINE_INFORMER_DCP_DCPERROR_HPP__

namespace eyeline{
namespace informer{
namespace dcp{

namespace DcpError{

static const int NotAuthorized=1;
static const int CommandHandlingError=2;
static const int AdminRoleRequired=3;
static const int TooManyDeliveries=4;
static const int AuthFailed=5;
static const int RequestNotFound=6;
static const int Expired=7;
static const int ItemNotFound=8;

};

}
}
}

#endif
