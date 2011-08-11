#include "MessageFlags.h"
#include "informer/io/HexDump.h"
#include "core/synchronization/MutexReportContentionRealization.h"

using namespace eyeline::informer;

int main()
{
    MessageFlagBuilder mfb;
    mfb.setTransactional( true );
    mfb.setUseDataSm( true );
    mfb.setSvcType("hello");
    mfb.setEyelineKeywordTLV("eyeline keyword");

    HexDump hd;
    {
        HexDump::string_type dump;
        hd.strdump( dump, mfb.getBuf(), mfb.getSize() );
        hd.hexdump( dump, mfb.getBuf(), mfb.getSize() );
        printf("builder: %s\n",hd.c_str(dump) );
    }

    MessageFlags mf;
    mf.reset(mfb);

    printf("empty: %d\n", mf.isEmpty());
    bool have, val;
    have = mf.hasTransactional(val);
    printf("transactional: %d %d\n", have, val);
    have = mf.hasUseDataSm(val);
    printf("usedatasm: %d %d\n", have, val);
    have = mf.hasReplaceIfPresent(val);
    printf("replaceifpresent: %d %d\n", have, val);
    have = mf.hasFlash(val);
    printf("flash: %d %d\n", have, val);
    const char* cval = mf.getSvcType();
    printf("svctype: %s\n", cval ? cval : "<none>" );
    DlvMode mode;
    have = mf.getDeliveryMode(mode);
    printf("deliverymode: %s\n", have ? "<none>" : dlvModeToString(mode));
    const char* tlvptr;
    size_t tlvsize = mf.getExtraTLV(tlvptr);
    if (tlvsize) {
        HexDump::string_type dump;
        hd.strdump(dump,tlvptr,tlvsize);
        hd.hexdump(dump,tlvptr,tlvsize);
        printf("extratlv: %s\n", hd.c_str(dump));
    } else {
        printf("extratlv: <none>\n");
    }
    return 0;
}
