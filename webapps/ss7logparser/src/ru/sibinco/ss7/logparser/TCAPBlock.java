package ru.sibinco.ss7.logparser;

import ru.sibinco.ss7.logparser.tcap.*;

import java.io.PrintWriter;
import java.util.HashMap;

/**
 * Created by: Serge Lugovoy
 * Date: 18.10.2004
 * Time: 11:43:57
 */
public class TCAPBlock extends LogBlock {
  static String primitiveNames[] = new String[256];
  static HashMap messageTypes = new HashMap();
  {
    for( int i = 0; i < 256; i++ ) primitiveNames[i] = String.valueOf(i);
    primitiveNames[6] = "T_BIND_REQ       ";
    primitiveNames[7] = "T_UNBIND_REQ     ";
    primitiveNames[8] = "T_UNI_REQ        ";
    primitiveNames[9] = "T_BEGIN_REQ      ";
    primitiveNames[10] = "T_CONTINUE_REQ   ";
    primitiveNames[11] = "T_END_REQ        ";
    primitiveNames[12] = "T_U_ABORT_REQ    ";
    primitiveNames[13] = "T_INVOKE_REQ     ";
    primitiveNames[14] = "T_RESULT_NL_REQ  ";
    primitiveNames[15] = "T_RESULT_L_REQ   ";
    primitiveNames[16] = "T_U_ERROR_REQ    ";
    primitiveNames[17] = "T_U_REJECT_REQ   ";
    primitiveNames[18] = "T_U_CANCEL_REQ   ";
    primitiveNames[45] = "T_PROCEED_REQ    ";
    primitiveNames[20] = "T_BIND_CONF      ";
    primitiveNames[21] = "T_UNI_IND        ";
    primitiveNames[22] = "T_BEGIN_IND      ";
    primitiveNames[23] = "T_CONTINUE_IND   ";
    primitiveNames[24] = "T_END_IND        ";
    primitiveNames[25] = "T_U_ABORT_IND    ";
    primitiveNames[26] = "T_P_ABORT_IND    ";
    primitiveNames[27] = "T_NOTICE_IND     ";
    primitiveNames[28] = "T_INVOKE_IND     ";
    primitiveNames[29] = "T_RESULT_NL_IND  ";
    primitiveNames[30] = "T_RESULT_L_IND   ";
    primitiveNames[31] = "T_U_ERROR_IND    ";
    primitiveNames[32] = "T_U_REJECT_IND   ";
    primitiveNames[33] = "T_L_REJECT_IND   ";
    primitiveNames[34] = "T_R_REJECT_IND   ";
    primitiveNames[35] = "T_L_CANCEL_IND   ";
    primitiveNames[36] = "T_STATE_IND      ";
    primitiveNames[42] = "T_UNITDATA_IND   ";

    messageTypes.put( new Integer(61), new MTUniParser());
    messageTypes.put( new Integer(62), new MTBeginParser());
    messageTypes.put( new Integer(65), new MTContinueParser());
    messageTypes.put( new Integer(64), new MTEndParser());
    messageTypes.put( new Integer(67), new MTAbortParser());
  }

  public void writePrimitiveInfo(PrintWriter pw) {
    pw.print("TCAP: ");
    pw.println(primitiveNames[primitive&0xFF]);
    MessageTypeParser parser = (MessageTypeParser) messageTypes.get( new Integer(Util.unsigned(data[0])) );
    if(parser != null ) {
      parser.decode(pw, data);
      pw.println();
    }
  }
}
