#define SINGLE_ROLLER
static char const ident[] = "$Id$";

#include "inman/storage/FileStorages.hpp"
using smsc::inman::filestore::RollingFileStorageParms;
using smsc::inman::filestore::InRollingFileStorage;
using smsc::inman::filestore::InFileStorageRoller;

#include "inman/storage/cdrutil.hpp"
using smsc::inman::cdr::CDRRecord;
using smsc::inman::cdr::_CDRRecordHeader_TEXT;

static char     _runService = 0;

extern "C" void sighandler(int signal)
{
    _runService = 0;
}


static const RollingFileStorageParms _smsc_BILLING_STORAGE_parms = {
    _CDRRecordHeader_TEXT, 0, 0, NULL
};
static const char* _LAST_BILLING_FILE_EXTENSION_1 = "inman.lst";
static const char* _PREV_BILLING_FILE_EXTENSION_1 = "inman.csv";

static const char* _LAST_BILLING_FILE_EXTENSION_2 = "smsc.lst";
static const char* _PREV_BILLING_FILE_EXTENSION_2 = "smsc.csv";

static const char _dflt_dir[] = "cdrs";


int main(int argc, char *argv[])
{
    char * _dflt_loc = (char*)&_dflt_dir[0];
    Logger::Init();
    Logger * uselog = Logger::getInstance("smsc.inman");

    if (argc > 1)
        _dflt_loc = argv[1];

    tzset();
    smsc_log_info(uselog, "RFSTst: storage dir %s", _dflt_loc);

    std::string location(_dflt_loc);
    unsigned long rollInterval1 = 15; //secs
    unsigned long rollInterval2 = 24; //secs

    //compose sample CDR record
    CDRRecord cdr;
    cdr._msgId = 555666;
    cdr._cdrType = CDRRecord::dpDiverted;
    cdr._bearer =  CDRRecord::dpSMS;
    cdr._mediaType = CDRRecord::dpText;
    cdr._routeId = "sibinco.sms > plmn.kem";
    cdr._serviceId = 1234;
    cdr._userMsgRef = 506;
    cdr._partsNum = 1;

    cdr._srcAdr = ".1.1.79139343290";
    cdr._srcIMSI = "250013900405871";
    cdr._srcMSC = ".1.1.79139860001";
    cdr._srcSMEid = "MAP_PROXY";

    cdr._dstAdr = ".1.1.79139859489";
    cdr._dstIMSI = "250013901464251";
    cdr._dstMSC = ".1.1.79139860001";
    cdr._dstSMEid = "DST_MAP_PROXY";
    cdr._dlvrRes = 1016;
    cdr._divertedAdr = ".1.1.79139859489";

    cdr._dpLength = 100;
    cdr._inBilled = false;

    //Open Rolling storages at same dir

    std::auto_ptr<InRollingFileStorage> fs1(new InRollingFileStorage(location,
                           _LAST_BILLING_FILE_EXTENSION_1,
                           _PREV_BILLING_FILE_EXTENSION_1, 0,
                            &_smsc_BILLING_STORAGE_parms, uselog));
    {
        int rval = fs1->RFSOpen();
        if (rval < 0)
            return -1;
        smsc_log_info(uselog, "FS1{%s, %s} opened%s",
                      _LAST_BILLING_FILE_EXTENSION_1,
                      _PREV_BILLING_FILE_EXTENSION_1,
                       rval  > 0 ? ", old files rolled" : "");
    }

    std::auto_ptr<InRollingFileStorage> fs2(new InRollingFileStorage(location,
                           _LAST_BILLING_FILE_EXTENSION_2,
                           _PREV_BILLING_FILE_EXTENSION_2, 0,
                            &_smsc_BILLING_STORAGE_parms, uselog));
    {
        int rval = fs2->RFSOpen();
        if (rval < 0)
            return -1;
        smsc_log_info(uselog, "FS2{%s, %s} opened%s",
                      _LAST_BILLING_FILE_EXTENSION_2,
                      _PREV_BILLING_FILE_EXTENSION_2,
                       rval  > 0 ? ", old files rolled" : "");
    }

    std::auto_ptr<InFileStorageRoller> roller1(new InFileStorageRoller(fs1.get(), rollInterval1));
    smsc_log_info(uselog, "RFSTst: Roller1 attached to FS1");
    roller1->Start();

#ifndef SINGLE_ROLLER
    std::auto_ptr<InFileStorageRoller> roller2(new InFileStorageRoller(fs2.get(), rollInterval2));
    smsc_log_info(uselog, "RFSTst: Roller2 attached to FS2");
    roller2->Start();
#else  /* SINGLE_ROLLER */
    roller1->attachRFS(fs2.get(), rollInterval2);
    smsc_log_info(uselog, "RFSTst: Roller1 attached to FS2");
#endif /* SINGLE_ROLLER */

    sigset(SIGTERM, sighandler);

    _runService = 1;
    while(_runService) {
        sleep(5);

        cdr._submitTime = time(NULL) - 5;
        cdr._finalTime = time(NULL);

        std::string rec;
        CDRRecord::csvEncode(cdr, rec);
        smsc_log_info(uselog, "RFSTst: Writing CDRs at %lu secs", cdr._finalTime);
        fs1->RFSWrite(rec.c_str(), rec.size());
        fs1->RFSFlush();
        fs2->RFSWrite(rec.c_str(), rec.size());
        fs2->RFSFlush();
    }
    smsc_log_info(uselog, "RFSTst: Stopping ..");
    roller1->Stop();
#ifndef SINGLE_ROLLER
    roller2->Stop();
#endif /* SINGLE_ROLLER */
    return 0;
}

