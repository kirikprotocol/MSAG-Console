#include <stdio.h>

#include "Commands.h"
#include "smpp/smpp_structures.h"
#include "profiler/profiler-types.hpp"
#include "smeman/smeinfo.h"
#include "core/buffers/Array.hpp"
#include "Interconnect.h"
#include "InterconnectManager.h"
#include "listeners/DlCommandListener.h"
#include "listeners/MemCommandListener.h"
#include "listeners/MscCommandListener.h"
#include "listeners/PrcCommandListener.h"
#include "listeners/ProfileCommandListener.h"
#include "listeners/SmeCommandListener.h"
#include "listeners/SbmCommandListener.h"

using namespace smsc::cluster;
using smsc::core::buffers::Array;

    template<class T>
    int fun(T t){
        printf("%d\n", t);
        return 0;
    };

    template<class C, class T1, class T2, class T3, class T4, class T5,
                        class T6, class T7, class T8, class T9, class T10,
                        class T11, class T12, class T13, class T14, class T15,
                        class T16, class T17>
    void checkCommand(T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, 
                      T6 t6, T7 t7, T8 t8, T9 t9, T10 t10,
                      T11 t11, T12 t12, T13 t13, T14 t14, T15 t15, T16 t16, T17 t17)
    {
        uint8_t *buffer = 0;

        uint32_t len;

        C cmd(t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13, t14, t15, t16, t17);

        buffer = (uint8_t*)cmd.serialize(len); 
        if(buffer){
            printf("serialize - Ok, len: %d\n", len);

            if(cmd.deserialize(buffer, len))
                printf("deserialize - Ok, len: %d\n", len);

            delete buffer;
        }

    };

    template<class C, class T1, class T2, class T3, class T4, class T5,
                        class T6, class T7, class T8, class T9, class T10,
                        class T11, class T12, class T13, class T14, class T15,
                        class T16, class T17>
    void sendCommand(T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, 
                      T6 t6, T7 t7, T8 t8, T9 t9, T10 t10,
                      T11 t11, T12 t12, T13 t13, T14 t14, T15 t15, T16 t16, T17 t17, Interconnect *master){
        C *cmd = new C(t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13, t14, t15, t16, t17);
        master->sendCommand((Command*)cmd);
    };

    template<class C, class T1, class T2, class T3, class T4, class T5>
    void checkCommand(T1 t1, T2 t2, T3 t3, T4 t4, T5 t5)
    {
        uint8_t *buffer = 0;

        uint32_t len;

        C cmd(t1, t2, t3, t4, t5);

        buffer = (uint8_t*)cmd.serialize(len); 
        if(buffer){
            printf("serialize - Ok, len: %d\n", len);

            if(cmd.deserialize(buffer, len))
                printf("deserialize - Ok, len: %d\n", len);

            delete buffer;
        }

    };

    template<class C, class T1, class T2, class T3, class T4, class T5>
    void sendCommand(T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, Interconnect *master){
        C *cmd = new C(t1, t2, t3, t4, t5);
        master->sendCommand((Command*)cmd);
    };

    template<class C, class T1, class T2, class T3, class T4>
    void checkCommand(T1 t1, T2 t2, T3 t3, T4 t4)
    {
        uint8_t *buffer = 0;

        uint32_t len;

        C cmd(t1, t2, t3, t4);

        buffer = (uint8_t*)cmd.serialize(len); 
        if(buffer){
            printf("serialize - Ok, len: %d\n", len);

            if(cmd.deserialize(buffer, len))
                printf("deserialize - Ok, len: %d\n", len);

            delete buffer;
        }

    };

    template<class C, class T1, class T2, class T3, class T4>
    void sendCommand(T1 t1, T2 t2, T3 t3, T4 t4, Interconnect *master){
        C *cmd = new C(t1, t2, t3, t4);
        master->sendCommand((Command*)cmd);
    };

    template<class C, class T1, class T2, class T3>
    void checkCommand(T1 t1, T2 t2, T3 t3)
    {
        uint8_t *buffer = 0;

        uint32_t len;

        C cmd(t1, t2, t3);

        buffer = (uint8_t*)cmd.serialize(len); 
        if(buffer){
            printf("serialize - Ok, len: %d\n", len);

            if(cmd.deserialize(buffer, len))
                printf("deserialize - Ok, len: %d\n", len);

            delete buffer;
        }

    };

    template<class C, class T1, class T2, class T3>
    void sendCommand(T1 t1, T2 t2, T3 t3, Interconnect *master){
        C *cmd = new C(t1, t2, t3);
        master->sendCommand((Command*)cmd);
    };

    template<class C, class T1, class T2>
    void checkCommand(T1 t1, T2 t2)
    {
        uint8_t *buffer = 0;

        uint32_t len;

        C cmd(t1, t2);

        buffer = (uint8_t*)cmd.serialize(len); 
        if(buffer){
            printf("serialize - Ok, len: %d\n", len);

            if(cmd.deserialize(buffer, len))
                printf("deserialize - Ok, len: %d\n", len);

            delete buffer;
        }

    };

    template<class C, class T1, class T2>
    void sendCommand(T1 t1, T2 t2, Interconnect *master){
        C *cmd = new C(t1, t2);
        master->sendCommand((Command*)cmd);
    };

    template<class C, class T>
    void checkCommand(T t)
    {
        uint8_t *buffer = 0;

        uint32_t len;

        C cmd(t);

        buffer = (uint8_t*)cmd.serialize(len); 
        if(buffer){
            printf("serialize - Ok, len: %d\n", len);

            if(cmd.deserialize(buffer, len))
                printf("deserialize - Ok, len: %d\n", len);

            delete buffer;
        }

    };

    template<class C, class T>
    void sendCommand(T t, Interconnect *master){
        C *cmd = new C(t);
        master->sendCommand((Command*)cmd);
    };

int main(int argc, char* argv[])
{

    // applyRoutesCmd
    ApplyRoutesCommand applyRoutesCmd;

    uint32_t len;
    uint8_t *buffer = (uint8_t*)applyRoutesCmd.serialize(len);
    if(!buffer)
        printf("\napplyRoutes:\nserialize - Ok, len: %d\n", len);

    buffer = 0;
    len = 0;

    if(applyRoutesCmd.deserialize(buffer, len))
        printf("deserialize - Ok, len: %d\n", len);

    buffer = new uint8_t[10];
    len = 10;

    if(!applyRoutesCmd.deserialize(buffer, len))
        printf("deserialize - failed, len %d\n\n", len);

    delete buffer;

    //AclAddAddressesCommand

    smsc::acls::AclIdent aclId = 10;
    std::vector<std::string> addr;
    addr.push_back("addr1");
    addr.push_back("addr2");

    AclAddAddressesCommand aclAddAddressesCmd(aclId, addr);

    buffer = (uint8_t*)aclAddAddressesCmd.serialize(len); 
    if(buffer){
        printf("\naclAddAddress:\nserialize - Ok, len: %d\n", len);

        if(aclAddAddressesCmd.deserialize(buffer, len))
            printf("deserialize - Ok, len: %d\n", len);

        delete buffer;
    }

    //AclCreateCommand

    std::string name = "name";
    std::string desc = "desc";
    std::string type = "type";
    std::vector<std::string> phones;
    phones.push_back("phone1");
    phones.push_back("phone2");

    printf("\naclCreateCommand:\n");
    AclCreateCommand aclCreateCmd(name, desc, type, phones);

    buffer = (uint8_t*)aclCreateCmd.serialize(len); 
    if(buffer){
        printf("serialize - Ok, len: %d\n", len);

        if(aclCreateCmd.deserialize(buffer, len))
            printf("deserialize - Ok, len: %d\n", len);

        delete buffer;
    }

    //AclRemoveAddressesCommand
    {
        uint8_t *buffer = 0;
        smsc::acls::AclIdent aclId = 1;
        std::vector<std::string> addresses;
        addresses.push_back("addr1");
        addresses.push_back("addr2");

        printf("\nAclRemoveAddressesCommand:\n");
        AclRemoveAddressesCommand cmd(aclId, addresses);

        buffer = (uint8_t*)cmd.serialize(len); 
        if(buffer){
            printf("serialize - Ok, len: %d\n", len);

            if(cmd.deserialize(buffer, len))
                printf("deserialize - Ok, len: %d\n", len);

            delete buffer;
        }

    }

    //AclRemoveCommand
    {
        uint8_t *buffer = 0;
        smsc::acls::AclIdent aclId = 52500;

        printf("\nAclRemoveCommand:\n");
        AclRemoveCommand cmd(aclId);

        buffer = (uint8_t*)cmd.serialize(len); 
        if(buffer){
            printf("serialize - Ok, len: %d\n", len);

            if(cmd.deserialize(buffer, len))
                printf("deserialize - Ok, len: %d\n", len);

            delete buffer;
        }

    }

    //AclUpdateInfoCommand
    {
        uint8_t *buffer = 0;

        smsc::acls::AclIdent aclId = 52550;
        std::string name = "name";
        std::string description = "desc";
        std::string cache_type = "type";

        printf("\nAclUpdateInfoCommand:\n");
        AclUpdateInfoCommand cmd(aclId, name, description, cache_type);

        buffer = (uint8_t*)cmd.serialize(len); 
        if(buffer){
            printf("serialize - Ok, len: %d\n", len);

            if(cmd.deserialize(buffer, len))
                printf("deserialize - Ok, len: %d\n", len);

            delete buffer;
        }

    }

    printf("\nDlAddCommand:\n");
    checkCommand<DlAddCommand>(52550, std::string("name"), std::string("owner"));

    printf("\nDlAlterCommand:\n");
    checkCommand<DlAlterCommand>(52550, std::string("name"));

    printf("\nDlDeleteCommand:\n");
    checkCommand<DlDeleteCommand>(std::string("name"));

    printf("\nMemAddMemberCommand:\n");
    checkCommand<MemAddMemberCommand>(std::string("name"), std::string("address"));

    printf("\nMemDeleteMemberCommand:\n");
    checkCommand<MemDeleteMemberCommand>(std::string("name"), std::string("address"));

    printf("\nMscBlockCommand:\n");
    checkCommand<MscBlockCommand>("mscNum");

    printf("\nMscClearCommand:\n");
    checkCommand<MscClearCommand>("mscNum");

    printf("\nMscRegistrateCommand:\n");
    checkCommand<MscRegistrateCommand>("mscNum");

    printf("\nMscUnregisterCommand:\n");
    checkCommand<MscUnregisterCommand>("mscNum");

    printf("\nPrcAddPrincipalCommand:\n");
    checkCommand<PrcAddPrincipalCommand>(10, 5329, "address");

    printf("\nPrcAlterPrincipalCommand:\n");
    checkCommand<PrcAlterPrincipalCommand>(1027, 5079, "addresses");

    printf("\nPrcDeletePrincipalCommand:\n");
    checkCommand<PrcDeletePrincipalCommand>("address");

    printf("\nProfileDeleteCommand:\n");
    checkCommand<ProfileDeleteCommand>(1, 50, "address");

    {
        uint8_t plan = 50;
        uint8_t type = 55;
        char address[] = "address";
        int codePage = smsc::smpp::DataCoding::SMSC7BIT;
        int reportOption = smsc::profiler::ProfileReportOptions::ReportNone;
        int hideOption = smsc::profiler::HideOption::hoDisabled;

        bool hideModifaible = true;
        bool divertModifaible = true;
        bool udhContact = false;
        bool translit = false;

        bool divertActive = true;
        bool divertActiveAbsent = true;
        bool divertActiveBlocked = false;
        bool divertActiveBarred = true;
        bool divertActiveCapacity = false;

        std::string local = "local";
        std::string divert = "divert";

        printf("\nProfileUpdateCommand:\n");
        checkCommand<ProfileUpdateCommand>(plan, type, address, codePage, reportOption, hideOption, hideModifaible,
                                                divertModifaible, udhContact, translit, divertActive, divertActiveAbsent,
                                                divertActiveBlocked, divertActiveBarred, divertActiveCapacity,
                                                local, divert);
    }

    printf("\nSbmAddSubmiterCommand:\n");
    checkCommand<SbmAddSubmiterCommand>("name", "address");

    printf("\nSbmDeleteSubmiterCommand:\n");
    checkCommand<SbmDeleteSubmiterCommand>("name", "address");

    {

        smsc::smeman::SmeInfo si;
        si.typeOfNumber = 1;
        si.numberingPlan = 2;
        si.interfaceVersion = 0x34;
        si.rangeOfAddress = ".*";
        si.systemType = "regular";
        si.password = "password";
        si.hostname = "hostname";
        si.port = 1;
        si.systemId = "systemId";
        si.priority = 1;
        si.SME_N = 1;
        si.disabled = true;
        si.wantAlias = true;
        si.forceDC = true;
        si.internal = false;
        si.bindMode = smsc::smeman::smeRX;
        si.receiptSchemeName = "receiptShemeName";
        si.timeout = 1;
        si.proclimit = 2;
        si.schedlimit = 3;
        si.providerId = 4;

        printf("\nSmeAddCommand:\n");
        checkCommand<SmeAddCommand>(si);

    }

    printf("\nSmeRemoveCommand:\n");
    checkCommand<SmeRemoveCommand>("systemId");

    {
        smsc::smeman::SmeInfo si;
        si.typeOfNumber = 1;
        si.numberingPlan = 2;
        si.interfaceVersion = 0x34;
        si.rangeOfAddress = ".*";
        si.systemType = "regular";
        si.password = "password";
        si.hostname = "hostname";
        si.port = 1;
        si.systemId = "systemId";
        si.priority = 1;
        si.SME_N = 1;
        si.disabled = true;
        si.wantAlias = true;
        si.forceDC = true;
        si.internal = false;
        si.bindMode = smsc::smeman::smeRX;
        si.receiptSchemeName = "receiptShemeName";
        si.timeout = 1;
        si.proclimit = 2;
        si.schedlimit = 3;
        si.providerId = 4;

        printf("\nSmeUpdateCommand:\n");
        checkCommand<SmeUpdateCommand>(si);
    }

    fun(1);

    Array<int> arr;
    for(int i=0; i<=20; i++){
        arr.Push(i);
    }
    
    /*int j;
    int res; 
    int count = arr.Count();
    for(int i=0; i<=count-1; i++){
        printf("count: %d\n", arr.Count());
        arr.Shift(j);
        printf("j: %d, count: %d\n", j, arr.Count());
    }
    arr.Clean();*/

    // Starts Interconnect manager
    printf("Interconnect manager starting...\n");
    InterconnectManager::init(MASTER, "m_ip", "s_ip", 1000);
    printf("Interconnect manager is started\n");

    printf("Instance is getting...\n");
    Interconnect * master = InterconnectManager::getInstance();
    printf("Instance is got\n");

    std::auto_ptr<DlCommandListener> dlCl ( new DlCommandListener(0) );
    master->addListener(DLADD_CMD, (CommandListener*)dlCl.get() );

    std::auto_ptr<MemCommandListener> memCl ( new MemCommandListener(0) );
    master->addListener(MEMADDMEMBER_CMD, (CommandListener*)memCl.get() );

    std::auto_ptr<MscCommandListener> mscCl ( new MscCommandListener() );
    master->addListener(MSCREGISTRATE_CMD, (CommandListener*)mscCl.get() );

    std::auto_ptr<PrcCommandListener> prcCl ( new PrcCommandListener(0) );
    master->addListener(PRCADDPRINCIPAL_CMD, (CommandListener*)prcCl.get() );

    std::auto_ptr<ProfileCommandListener> proCl ( new ProfileCommandListener(0) );
    master->addListener(PROFILEUPDATE_CMD, (CommandListener*)proCl.get() );

    std::auto_ptr<SmeCommandListener> smeCl ( new SmeCommandListener(0) );
    master->addListener(SMEADD_CMD, (CommandListener*)smeCl.get() );

    std::auto_ptr<SbmCommandListener> sbmCl ( new SbmCommandListener(0) );
    master->addListener(SBMADDSUBMITER_CMD, (CommandListener*)sbmCl.get() );

    sleep(1);

    // Sends commands

    /*printf("\n============== Sends commands ===============\n");

    {
        smsc::smeman::SmeInfo si;
        si.typeOfNumber = 1;
        si.numberingPlan = 2;
        si.interfaceVersion = 0x34;
        si.rangeOfAddress = ".*";
        si.systemType = "regular";
        si.password = "password";
        si.hostname = "hostname";
        si.port = 1;
        si.systemId = "systemId";
        si.priority = 1;
        si.SME_N = 1;
        si.disabled = true;
        si.wantAlias = true;
        si.forceDC = true;
        si.internal = false;
        si.bindMode = smsc::smeman::smeRX;
        si.receiptSchemeName = "receiptShemeName";
        si.timeout = 1;
        si.proclimit = 2;
        si.schedlimit = 3;
        si.providerId = 4;

        printf("\nSmeUpdateCommand:\n");
        sendCommand<SmeUpdateCommand>(si, master);
    }

    printf("\nDlAddCommand:\n");
    sendCommand<DlAddCommand>(52550, std::string("name"), std::string("owner"), master);

    printf("\nDlAlterCommand:\n");
    sendCommand<DlAlterCommand>(52550, std::string("name"), master);

    printf("\nDlDeleteCommand:\n");
    sendCommand<DlDeleteCommand>(std::string("name"), master);

    printf("\nMemAddMemberCommand:\n");
    sendCommand<MemAddMemberCommand>(std::string("name"), std::string("address"), master);

    printf("\nMemDeleteMemberCommand:\n");
    sendCommand<MemDeleteMemberCommand>(std::string("name"), std::string("address"), master);

    printf("\nMscBlockCommand:\n");
    sendCommand<MscBlockCommand>("mscNum", master);

    printf("\nMscClearCommand:\n");
    sendCommand<MscClearCommand>("mscNum", master);

    printf("\nMscRegistrateCommand:\n");
    sendCommand<MscRegistrateCommand>("mscNum", master);

    printf("\nMscUnregisterCommand:\n");
    sendCommand<MscUnregisterCommand>("mscNum", master);

    printf("\nPrcAddPrincipalCommand:\n");
    sendCommand<PrcAddPrincipalCommand>(10, 5329, "address", master);

    printf("\nPrcAlterPrincipalCommand:\n");
    sendCommand<PrcAlterPrincipalCommand>(1027, 5079, "addresses", master);

    printf("\nPrcDeletePrincipalCommand:\n");
    sendCommand<PrcDeletePrincipalCommand>("address", master);

    printf("\nProfileDeleteCommand:\n");
    sendCommand<ProfileDeleteCommand>(1, 50, "address", master);

    {
        uint8_t plan = 50;
        uint8_t type = 55;
        char address[] = "address";
        int codePage = smsc::smpp::DataCoding::SMSC7BIT;
        int reportOption = smsc::profiler::ProfileReportOptions::ReportNone;
        int hideOption = smsc::profiler::HideOption::hoDisabled;

        bool hideModifaible = true;
        bool divertModifaible = true;
        bool udhContact = false;
        bool translit = false;

        bool divertActive = true;
        bool divertActiveAbsent = true;
        bool divertActiveBlocked = false;
        bool divertActiveBarred = true;
        bool divertActiveCapacity = false;

        std::string local = "local";
        std::string divert = "divert";

        printf("\nProfileUpdateCommand:\n");
        sendCommand<ProfileUpdateCommand>(plan, type, address, codePage, reportOption, hideOption, hideModifaible,
                                                divertModifaible, udhContact, translit, divertActive, divertActiveAbsent,
                                                divertActiveBlocked, divertActiveBarred, divertActiveCapacity,
                                                local, divert, master);
    }

    printf("\nSbmAddSubmiterCommand:\n");
    sendCommand<SbmAddSubmiterCommand>("name", "address", master);

    printf("\nSbmDeleteSubmiterCommand:\n");
    sendCommand<SbmDeleteSubmiterCommand>("name", "address", master);

    {

        smsc::smeman::SmeInfo si;
        si.typeOfNumber = 1;
        si.numberingPlan = 2;
        si.interfaceVersion = 0x34;
        si.rangeOfAddress = ".*";
        si.systemType = "regular";
        si.password = "password";
        si.hostname = "hostname";
        si.port = 1;
        si.systemId = "systemId";
        si.priority = 1;
        si.SME_N = 1;
        si.disabled = true;
        si.wantAlias = true;
        si.forceDC = true;
        si.internal = false;
        si.bindMode = smsc::smeman::smeRX;
        si.receiptSchemeName = "receiptShemeName";
        si.timeout = 1;
        si.proclimit = 2;
        si.schedlimit = 3;
        si.providerId = 4;

        printf("\nSmeAddCommand:\n");
        sendCommand<SmeAddCommand>(si, master);

    }

    printf("\nSmeRemoveCommand:\n");
    sendCommand<SmeRemoveCommand>("systemId", master);

    {
        smsc::smeman::SmeInfo si;
        si.typeOfNumber = 1;
        si.numberingPlan = 2;
        si.interfaceVersion = 0x34;
        si.rangeOfAddress = ".*";
        si.systemType = "regular";
        si.password = "password";
        si.hostname = "hostname";
        si.port = 1;
        si.systemId = "systemId";
        si.priority = 1;
        si.SME_N = 1;
        si.disabled = true;
        si.wantAlias = true;
        si.forceDC = true;
        si.internal = false;
        si.bindMode = smsc::smeman::smeRX;
        si.receiptSchemeName = "receiptShemeName";
        si.timeout = 1;
        si.proclimit = 2;
        si.schedlimit = 3;
        si.providerId = 4;

        printf("\nSmeUpdateCommand:\n");
        sendCommand<SmeUpdateCommand>(si, master);
    }*/

    printf("Active\n");
    master->activate();

    sleep(3);

    InterconnectManager::shutdown();
    printf("Interconnect manager is stoped\n");

    printf("Okkey\n");

    return 0;
}
