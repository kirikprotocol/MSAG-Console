package ru.sibinco.ss7.logparser;

import java.io.PrintWriter;

/**
 * Created by: Serge Lugovoy
 * Date: 18.10.2004
 * Time: 16:02:21
 */
public class SCCPBlock extends LogBlock {
  static String primitiveNames[] = new String[256];
  {
    for( int i = 0; i < 256; i++ ) primitiveNames[i] = String.valueOf(i);
    primitiveNames[5] = "N_BIND_REQ         ";
    primitiveNames[6] = "N_UNBIND_REQ       ";
    primitiveNames[7] = "N_UNITDATA_REQ     ";
    primitiveNames[13] = "N_CONNECT_REQ      ";
    primitiveNames[14] = "N_CONNECT_RESP     ";
    primitiveNames[15] = "N_DISCONNECT_REQ   ";
    primitiveNames[16] = "N_DATA_REQ         ";
    primitiveNames[19] = "N_BIND_CONF        ";
    primitiveNames[20] = "N_NOTICE_IND       ";
    primitiveNames[21] = "N_UNITDATA_IND     ";
    primitiveNames[22] = "N_CONNECT_IND      ";
    primitiveNames[23] = "N_CONNECT_CONF     ";
    primitiveNames[24] = "N_DATA_IND         ";
    primitiveNames[25] = "N_DISCONNECT_IND   ";
    primitiveNames[26] = "N_STATE_IND        ";
  }

  public void writePrimitiveInfo(PrintWriter pw) {
    pw.print("SCCP: ");
    pw.println(primitiveNames[primitive&0xFF]);
  }
}
