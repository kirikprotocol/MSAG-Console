#ifndef __SMSC_SMPP_SMPP_ERR_H__
namespace smsc{
namespace smpp{

static const char* const smpperrstr(const int smpp_command_status)
{
  //Command_Status Error Codes
  #define addcs(cs, errid, strerr) case(cs): return errid " " strerr;
  switch(smpp_command_status){
    default: return "Unknown SMPP command_status";
    addcs(0x00000000, "ESME_ROK", "No Error");
    addcs(0x00000001, "ESME_RINVMSGLEN", "");
    addcs(0x00000002, "ESME_RINVCMDLEN", "");
    addcs(0x00000003, "ESME_RINVCMDID", "");
    addcs(0x00000004, "ESME_RINVBNDSTS", "");
    addcs(0x00000005, "ESME_RALYBND", "");
    addcs(0x00000006, "ESME_RINVPRTFLG", "");
    addcs(0x00000007, "ESME_RINVREGDLVFLG", "");
    addcs(0x00000008, "ESME_RSYSERR", "");
    addcs(0x0000000A, "ESME_RINVSRCADR", "Invalid Source Address");
    addcs(0x0000000B, "ESME_RINVDSTADR", "Invalid Dest Addr");
    addcs(0x0000000C, "ESME_RINVMSGID", "Message ID is invalid");
    addcs(0x0000000D, "ESME_RBINDFAIL", "Bind Failed");
    addcs(0x0000000E, "ESME_RINVPASWD", "Invalid Password");
    addcs(0x0000000F, "ESME_RINVSYSID", "Invalid System ID");
    addcs(0x00000011, "ESME_RCANCELFAIL", "");
    addcs(0x00000013, "ESME_RREPLACEFAIL", "");
    addcs(0x00000014, "ESME_RMSGQFUL", "Message Queue Full");
    addcs(0x00000015, "ESME_RINVSERTYP", "");
    addcs(0x00000019, "ESME_RADDCUSTFAIL", "Failed to Add Customer");
    addcs(0x0000001A, "ESME_RDELCUSTFAIL", "Failed to delete Customer");
    addcs(0x0000001B, "ESME_RMODCUSTFAIL", "Failed to modify customer");
    addcs(0x0000001C, "ESME_RENQCUSTFAIL", "Failed to Enquire Customer");
    addcs(0x0000001D, "ESME_RINVCUSTID", "Invalid Customer ID");
    addcs(0x0000001F, "ESME_RINVCUSTNAME", "Invalid Customer Name");
    addcs(0x00000021, "ESME_RINVCUSTADR", "Invalid Customer Address");
    addcs(0x00000022, "ESME_RINVADR", "Invalid Address");
    addcs(0x00000023, "ESME_RCUSTEXIST", "Customer Exists");
    addcs(0x00000024, "ESME_RCUSTNOTEXIST", "Customer does not exist");
    addcs(0x00000026, "ESME_RADDDLFAIL", "Failed to Add DL");
    addcs(0x00000027, "ESME_RMODDLFAIL", "Failed to modify DL");
    addcs(0x00000028, "ESME_RDELDLFAIL", "Failed to Delete DL");
    addcs(0x00000029, "ESME_RVIEWDLFAIL", "Failed to View DL");
    addcs(0x00000030, "ESME_RLISTDLSFAIL", "Failed to list DLs");
    addcs(0x00000031, "ESME_RPARAMRETFAIL", "Param Retrieve Failed");
    addcs(0x00000032, "ESME_RINVPARAM", "Invalid Param");
    addcs(0x00000033, "ESME_RINVNUMDESTS", "");
    addcs(0x00000034, "ESME_RINVDLNAME", "");
    addcs(0x00000035, "ESME_RINVDLMEMBDESC", "Invalid DL Member Description");
    addcs(0x00000038, "ESME_RINVDLMEMBTYP", "Invalid DL Member Type");
    addcs(0x00000039, "ESME_RINVDLMODOPT", "Invalid DL Modify Option");
    addcs(0x00000040, "ESME_RINVDESTFLAG", "");
    addcs(0x00000042, "ESME_RINVSUBREP", "");
    addcs(0x00000043, "ESME_RINVESMCLASS", "");
    addcs(0x00000044, "ESME_RCNTSUBDL", "");
    addcs(0x00000045, "ESME_RSUBMITFAIL", "");
    addcs(0x00000048, "ESME_RINVSRCTON", "");
    addcs(0x00000049, "ESME_RINVSRCNPI", "");
    addcs(0x00000050, "ESME_RINVDSTTON", "");
    addcs(0x00000051, "ESME_RINVDSTNPI", "");
    addcs(0x00000053, "ESME_RINVSYSTYP", "");
    addcs(0x00000054, "ESME_RINVREPFLAG", "");
    addcs(0x00000055, "ESME_RINVNUMMSGS", "");
    addcs(0x00000058, "ESME_RTHROTTLED", "");
    addcs(0x00000059, "ESME_RPROVNOTALLWD", "Provisioning Not Allowed");
    addcs(0x00000061, "ESME_RINVSCHED", "");
    addcs(0x00000062, "ESME_RINVEXPIRY", "");
    addcs(0x00000063, "ESME_RINVDFTMSGID", "");
    addcs(0x00000064, "ESME_RX_T_APPN", "");
    addcs(0x00000065, "ESME_RX_P_APPN", "");
    addcs(0x00000066, "ESME_RX_R_APPN", "");
    addcs(0x00000067, "ESME_RQUERYFAIL", "");
    addcs(0x00000080, "ESME_RINVPGCUSTID", "Paging Customer ID Invalid No such subscriber");
    addcs(0x00000081, "ESME_RINVPGCUSTIDLEN", "Paging Customer ID length Invalid");
    addcs(0x00000082, "ESME_RINVCITYLEN", "City Length Invalid");
    addcs(0x00000083, "ESME_RINVSTATELEN", "State Length Invalid");
    addcs(0x00000084, "ESME_RINVZIPPREFIXLEN", "Zip Prefix Length Invalid");
    addcs(0x00000085, "ESME_RINVZIPPOSTFIXLEN", "Zip Postfix Length Invalid");
    addcs(0x00000086, "ESME_RINVMINLEN", "MIN Length Invalid");
    addcs(0x00000087, "ESME_RINVMIN", "MIN Invalid (i.e. No such MIN)");
    addcs(0x00000088, "ESME_RINVPINLEN", "PIN Length Invalid");
    addcs(0x00000089, "ESME_RINVTERMCODELEN", "Terminal Code Length Invalid");
    addcs(0x0000008A, "ESME_RINVCHANNELLEN", "Channel Length Invalid");
    addcs(0x0000008B, "ESME_RINVCOVREGIONLEN", "Coverage Region Length Invalid");
    addcs(0x0000008C, "ESME_RINVCAPCODELEN", "Cap Code Length Invalid");
    addcs(0x0000008D, "ESME_RINVMDTLEN", "Message delivery time Length Invalid");
    addcs(0x0000008E, "ESME_RINVPRIORMSGLEN", "Priority Message Length Invalid");
    addcs(0x0000008F, "ESME_RINVPERMSGLEN", "Periodic Messages Length Invalid");
    addcs(0x00000090, "ESME_RINVPGALERTLEN", "Paging Alerts Length Invalid");
    addcs(0x00000091, "ESME_RINVSMUSERLEN", "Short Message User Group Length Invalid");
    addcs(0x00000092, "ESME_RINVRTDBLEN", "Real Time Data broadcasts Length Invalid");
    addcs(0x00000093, "ESME_RINVREGDELLEN", "Registered Delivery Lenght Invalid");
    addcs(0x00000094, "ESME_RINVMSGDISTLEN", "Message Distribution Lenght Invalid");
    addcs(0x00000095, "ESME_RINVPRIORMSG", "Priority Message Length Invalid");
    addcs(0x00000096, "ESME_RINVMDT", "Message delivery time Invalid");
    addcs(0x00000097, "ESME_RINVPERMSG", "Periodic Messages Invalid");
    addcs(0x00000098, "ESME_RINVMSGDIST", "Message Distribution Invalid");
    addcs(0x00000099, "ESME_RINVPGALERT", "Paging Alerts Invalid");
    addcs(0x0000009A, "ESME_RINVSMUSER", "Short Message User Group Invalid");
    addcs(0x0000009B, "ESME_RINVRTDB", "Real Time Data broadcasts Invalid");
    addcs(0x0000009C, "ESME_RINVREGDEL", "Registered Delivery Invalid");
    addcs(0x0000009D, "ESME_RINVOPTPARSTREAM", "KIF IW Field out of data");
    addcs(0x0000009E, "ESME_ROPTPARNOTALLWD", "Optional Parameter not allowed");
    addcs(0x0000009F, "ESME_RINVOPTPARLEN", "Invalid Optional Parameter Length");
    addcs(0x000000C3, "ESME_RMISSINGOPTPARAM", "");
    addcs(0x000000C4, "ESME_RINVOPTPARAMVAL", "");
    addcs(0x000000FE, "ESME_RDELIVERYFAILURE", "");
    addcs(0x000000FF, "ESME_RUNKNOWNERR", "");
  }
  #undef addcs
}

};//smpp
};//smsc

#endif
