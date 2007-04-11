static char const ident[] = "$Id$";
#include "ACRepo.hpp"

namespace smsc{namespace mtsmsme{namespace processor{

static unsigned long           null_buf[] = {0,0,0,0,0,0, 0,0};
AC null_ac        = AC(null_buf,sizeof(null_buf)/sizeof(unsigned long));

static unsigned long sm_mt_relay_v1_buf[] = {0,4,0,0,1,0,25,1};
static unsigned long sm_mt_relay_v2_buf[] = {0,4,0,0,1,0,25,2};
static unsigned long sm_mt_relay_v3_buf[] = {0,4,0,0,1,0,25,3};
AC sm_mt_relay_v1 = AC(sm_mt_relay_v1_buf,sizeof(sm_mt_relay_v1_buf)/sizeof(unsigned long));
AC sm_mt_relay_v2 = AC(sm_mt_relay_v2_buf,sizeof(sm_mt_relay_v2_buf)/sizeof(unsigned long));
AC sm_mt_relay_v3 = AC(sm_mt_relay_v3_buf,sizeof(sm_mt_relay_v3_buf)/sizeof(unsigned long));

static unsigned long net_loc_upd_v1_buf[] = {0,4,0,0,1,0, 1,1};
static unsigned long net_loc_upd_v2_buf[] = {0,4,0,0,1,0, 1,2};
static unsigned long net_loc_upd_v3_buf[] = {0,4,0,0,1,0, 1,3};
AC net_loc_upd_v1 = AC(net_loc_upd_v1_buf,sizeof(net_loc_upd_v1_buf)/sizeof(unsigned long));
AC net_loc_upd_v2 = AC(net_loc_upd_v2_buf,sizeof(net_loc_upd_v2_buf)/sizeof(unsigned long));
AC net_loc_upd_v3 = AC(net_loc_upd_v3_buf,sizeof(net_loc_upd_v3_buf)/sizeof(unsigned long));

static unsigned long net_loc_cancel_v1_buf[] = {0,4,0,0,1,0,2,1};
static unsigned long net_loc_cancel_v2_buf[] = {0,4,0,0,1,0,2,2};
static unsigned long net_loc_cancel_v3_buf[] = {0,4,0,0,1,0,2,3};
AC net_loc_cancel_v1 = AC(net_loc_cancel_v1_buf,sizeof(net_loc_cancel_v1_buf)/sizeof(unsigned long));
AC net_loc_cancel_v2 = AC(net_loc_cancel_v2_buf,sizeof(net_loc_cancel_v2_buf)/sizeof(unsigned long));
AC net_loc_cancel_v3 = AC(net_loc_cancel_v3_buf,sizeof(net_loc_cancel_v3_buf)/sizeof(unsigned long));

static uint8_t magic0780[] = {0x80};
BIT_STRING_t tcapversion = {magic0780,sizeof(magic0780),0x07,};

static uint8_t magic773[] = {0x00,0x11,0x86,0x05,0x01,0x01,0x01};
ASN__PRIMITIVE_TYPE_t pduoid = {magic773,sizeof(magic773)};

/* namespace processor */ } /* namespace mtsmsme */ } /* namespace smsc */ }
