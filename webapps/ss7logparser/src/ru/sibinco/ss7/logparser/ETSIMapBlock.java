package ru.sibinco.ss7.logparser;

import java.io.PrintWriter;

/**
 * Created by: Serge Lugovoy
 * Date: 18.10.2004
 * Time: 11:38:40
 */
public class ETSIMapBlock extends LogBlock {
  static String primitiveNames[] = new String[256];
  static {
    for( int i = 0; i < 256; i++ ) primitiveNames[i] = String.valueOf(i);
    primitiveNames[121] = "MAP_BIND_REQ              ";
    primitiveNames[122] = "MAP_BIND_CONF             ";
    primitiveNames[123] = "MAP_UNBIND_REQ            ";
    primitiveNames[124] = "MAP_BEGIN_REQ             ";
    primitiveNames[125] = "MAP_BEGIN_IND             ";
    primitiveNames[126] = "MAP_END_REQ               ";
    primitiveNames[127] = "MAP_ERR_REQ               ";
    primitiveNames[128] = "MAP_ERR_IND               ";
    primitiveNames[129] = "MAP_DEL_SM_REQ            ";
    primitiveNames[130] = "MAP_RPT_SM_IND            ";
    primitiveNames[131] = "MAP_SUBMIT_SM_IND         ";
    primitiveNames[132] = "MAP_SUBMIT_SM_CONF        ";
    primitiveNames[133] = "MAP_ALERT_IND             ";
    primitiveNames[134] = "MAP_OPEN_REQ              ";
    primitiveNames[135] = "MAP_OPEN_CONF             ";
    primitiveNames[136] = "MAP_OPEN_IND              ";
    primitiveNames[137] = "MAP_OPEN_RSP              ";
    primitiveNames[138] = "MAP_CLOSE_REQ             ";
    primitiveNames[139] = "MAP_CLOSE_IND             ";
    primitiveNames[140] = "MAP_DELIMIT_REQ           ";
    primitiveNames[141] = "MAP_DELIMIT_IND           ";
    primitiveNames[142] = "MAP_U_ABORT_REQ           ";
    primitiveNames[143] = "MAP_U_ABORT_IND           ";
    primitiveNames[144] = "MAP_P_ABORT_IND           ";
    primitiveNames[145] = "MAP_NOTICE_IND            ";
    primitiveNames[146] = "MAP_PROC_USSD_REQ         ";
    primitiveNames[147] = "MAP_PROC_USSD_CONF        ";
    primitiveNames[148] = "MAP_PROC_USSD_IND         ";
    primitiveNames[149] = "MAP_PROC_USSD_RSP         ";
    primitiveNames[150] = "MAP_USSD_REQ              ";
    primitiveNames[151] = "MAP_USSD_CONF             ";
    primitiveNames[152] = "MAP_USSD_IND              ";
    primitiveNames[153] = "MAP_USSD_RSP              ";
    primitiveNames[154] = "MAP_USSD_NOTIFY_REQ       ";
    primitiveNames[155] = "MAP_USSD_NOTIFY_CONF      ";
    primitiveNames[156] = "MAP_USSD_NOTIFY_IND       ";
    primitiveNames[157] = "MAP_USSD_NOTIFY_RSP       ";
    primitiveNames[158] = "MAP_SND_RINFO_SM_REQ      ";
    primitiveNames[159] = "MAP_SND_RINFO_SM_CONF     ";
    primitiveNames[160] = "MAP_SND_RINFO_SM_IND      ";
    primitiveNames[161] = "MAP_SND_RINFO_SM_RSP      ";
    primitiveNames[162] = "MAP_FWD_SM_REQ            ";
    primitiveNames[163] = "MAP_FWD_SM_CONF           ";
    primitiveNames[164] = "MAP_FWD_SM_IND            ";
    primitiveNames[165] = "MAP_FWD_SM_RSP            ";
    primitiveNames[166] = "MAP_RPT_SM_DEL_REQ        ";
    primitiveNames[167] = "MAP_RPT_SM_DEL_CONF       ";
    primitiveNames[168] = "MAP_RPT_SM_DEL_IND        ";
    primitiveNames[169] = "MAP_RPT_SM_DEL_RSP        ";
    primitiveNames[170] = "MAP_RDY_FOR_SM_REQ        ";
    primitiveNames[171] = "MAP_RDY_FOR_SM_CONF       ";
    primitiveNames[172] = "MAP_RDY_FOR_SM_IND        ";
    primitiveNames[173] = "MAP_RDY_FOR_SM_RSP        ";
    primitiveNames[174] = "MAP_ALERT_SC_REQ          ";
    primitiveNames[175] = "MAP_ALERT_SC_CONF         ";
    primitiveNames[176] = "MAP_ALERT_SC_IND          ";
    primitiveNames[177] = "MAP_ALERT_SC_RSP          ";
    primitiveNames[178] = "MAP_INFORM_SC_REQ         ";
    primitiveNames[179] = "MAP_INFORM_SC_IND         ";
    primitiveNames[180] = "MAP_STATE_IND             ";
    primitiveNames[181] = "MAP_EMAP_USSD_REQ         ";
    primitiveNames[182] = "MAP_EMAP_USSD_CONF        ";
    primitiveNames[183] = "MAP_EMAP_USSD_NOTIFY_REQ  ";
    primitiveNames[184] = "MAP_EMAP_USSD_NOTIFY_CONF ";
    primitiveNames[185] = "MAP_V1_FWD_SM_REQ          ";
    primitiveNames[186] = "MAP_V1_FWD_SM_IND          ";
    primitiveNames[187] = "MAP_V1_FWD_SM_RSP          ";
    primitiveNames[188] = "MAP_V1_FWD_SM_CONF         ";
    primitiveNames[189] = "MAP_V1_RPT_SM_DEL_REQ      ";
    primitiveNames[190] = "MAP_V1_RPT_SM_DEL_CONF     ";
    primitiveNames[191] = "MAP_V1_ALERT_SC_IND        ";
    primitiveNames[192] = "MAP_V1_SND_RINFO_SM_CONF   ";
    primitiveNames[193] = "MAP_V1_SND_RINFO_SM_REQ    ";
    primitiveNames[194] = "MAP_GET_AC_VERSION_REQ     ";
    primitiveNames[195] = "MAP_GET_AC_VERSION_CONF    ";
    primitiveNames[196] = "MAP_FWD_SM_MO_REQ          ";
    primitiveNames[197] = "MAP_FWD_SM_MO_CONF         ";
    primitiveNames[200] = "MAP_ANY_TIME_INTERROGATION_REQ          ";
    primitiveNames[201] = "MAP_ANY_TIME_INTERROGATION_IND          ";
    primitiveNames[202] = "MAP_ANY_TIME_INTERROGATION_CONF         ";
    primitiveNames[203] = "MAP_ANY_TIME_INTERROGATION_RESP         ";
    primitiveNames[204] = "MAP_V3_SEND_ROUTING_INFO_FOR_LCS_REQ    ";
    primitiveNames[205] = "MAP_V3_SEND_ROUTING_INFO_FOR_LCS_CONF   ";
    primitiveNames[206] = "MAP_V3_PROVIDE_SUBSCRIBER_LOCATION_REQ  ";
    primitiveNames[207] = "MAP_V3_PROVIDE_SUBSCRIBER_LOCATION_CONF ";
    primitiveNames[208] = "MAP_V3_SUBSCRIBER_LOCATION_REPORT_IND   ";
    primitiveNames[209] = "MAP_V3_SUBSCRIBER_LOCATION_REPORT_RESP  ";
    primitiveNames[210] = "MAP_V3_FWD_SM_MT_REQ                    ";
    primitiveNames[211] = "MAP_V3_FWD_SM_MT_CONF                   ";
    primitiveNames[212] = "MAP_V3_FWD_SM_MO_IND                    ";
    primitiveNames[213] = "MAP_V3_FWD_SM_MO_RSP                    ";
    primitiveNames[214] = "MAP_V3_SND_RINFO_SM_REQ                 ";
    primitiveNames[215] = "MAP_V3_SND_RINFO_SM_CONF                ";
    primitiveNames[216] = "MAP_V3_RPT_SM_DEL_REQ                   ";
    primitiveNames[217] = "MAP_V3_RPT_SM_DEL_CONF                  ";
    primitiveNames[218] = "MAP_V3_INFORM_SC_IND                    ";
    primitiveNames[219] = "MAP_V3_ALERT_SC_IND                     ";
    primitiveNames[220] = "MAP_V3_ALERT_SC_RSP                     ";
    primitiveNames[221] = "MAP_BEGIN_SUB_ACTIVITY_REQ              ";
    primitiveNames[222] = "MAP_BEGIN_SUB_ACTIVITY_CONF             ";
    primitiveNames[223] = "MAP_BEGIN_SUB_ACTIVITY_IND              ";
    primitiveNames[224] = "MAP_BEGIN_SUB_ACTIVITY_RSP              ";
    primitiveNames[225] = "MAP_PROC_USSD_DATA_REQ                  ";
    primitiveNames[226] = "MAP_PROC_USSD_DATA_CONF                 ";
    primitiveNames[227] = "MAP_PROC_USSD_DATA_IND                  ";
    primitiveNames[228] = "MAP_PROC_USSD_DATA_RSP                  ";
    primitiveNames[229] = "MAP_V3_PROVIDE_SUBSCRIBER_INFO_REQ      ";
    primitiveNames[230] = "MAP_V3_PROVIDE_SUBSCRIBER_INFO_CONF     ";
    primitiveNames[231] = "MAP_V2_SEND_IMSI_REQ                    ";
    primitiveNames[232] = "MAP_V2_SEND_IMSI_CONF                   ";
    primitiveNames[233] = "MAP_V1_E_ENHANCED_IMEI_CHECK_IND        ";
    primitiveNames[234] = "MAP_V1_E_ENHANCED_IMEI_CHECK_RESP       ";
  }

  byte ssn;
  short dialogueId;
  public void init(int type, long time, boolean sent, int sender, int receiver, int primitive, int size, byte data[]) {
    super.init(type, time, sent, sender, receiver, primitive, size, data);
    ssn = data[1];
    dialogueId = (short)(((((int)data[3])&0xFF)<<8)|(((int)data[2])&0xFF));
  }

  public void writePrimitiveInfo(PrintWriter pw) {
    pw.print("MAP: ");
    pw.println(primitiveNames[primitive&0xFF]);
    pw.print("SSN: ");
    pw.print(((int)ssn)&0xFF);
    pw.print(" DLG: 0x");
    pw.println(Integer.toHexString(((int)dialogueId)&0xFFFF));
  }
}
