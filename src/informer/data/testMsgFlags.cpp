#include "MessageFlags.h"
#include "informer/io/HexDump.h"
#include "informer/io/IOConverter.h"
#include "core/synchronization/MutexReportContentionRealization.h"

using namespace eyeline::informer;


void showflags( const MessageFlags& mf )
{
    HexDump hd;
    {
        HexDump::string_type dump;
        hd.hexdump(dump,mf.buf(),mf.bufsize());
        hd.strdump(dump,mf.buf(),mf.bufsize());
        printf("flags buf: %s\n",hd.c_str(dump));
    }

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
        hd.hexdump(dump,tlvptr,tlvsize);
        hd.strdump(dump,tlvptr,tlvsize);
        printf("extratlv: %s\n", hd.c_str(dump));
    } else {
        printf("extratlv: <none>\n");
    }
}


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
        hd.hexdump( dump, mfb.getBuf(), mfb.getSize() );
        hd.strdump( dump, mfb.getBuf(), mfb.getSize() );
        printf("builder: %s\n",hd.c_str(dump) );
    }

    MessageFlags mf;
    mf.reset(mfb);
    showflags(mf);

    char storebuf[300];
    {
        ToBuf tb(storebuf,sizeof(storebuf));
        tb.setHexCString(mf.buf(),mf.bufsize());
        HexDump::string_type dump;
        hd.hexdump( dump, storebuf, mf.bufsize()*2+1 );
        hd.strdump( dump, storebuf, mf.bufsize()*2+1 );
        printf("storebuf: %s\n",hd.c_str(dump) );
    }

    FromBuf fb(storebuf,mf.bufsize()*2+1);
    MessageFlags(fb.getCString()).swap(mf);
    showflags(mf);
    return 0;
}
