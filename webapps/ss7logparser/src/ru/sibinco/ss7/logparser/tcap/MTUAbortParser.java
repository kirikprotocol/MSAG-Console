package ru.sibinco.ss7.logparser.tcap;

import ru.sibinco.ss7.logparser.Util;

import java.io.PrintWriter;

/**
 * Created by: Serge Lugovoy
 * Date: 18.10.2004
 * Time: 16:49:31
 */
public class MTUAbortParser extends MessageTypeParser {
  DialoguePortionParser dialoguePortionParser = new DialoguePortionParser();
  DestTransactionPortionParser destTransactionPortionParser = new DestTransactionPortionParser();

  public MTUAbortParser() {
    mtt = "UABORT";
  }

  public void decode(PrintWriter pw, byte data[]) {
    super.decode(pw, data);
    int pos = 2;
    pos = destTransactionPortionParser.decode(pw, data, pos);
    pos = dialoguePortionParser.decode(pw,data,pos);
    pw.print(" info: "+Util.hex(data, pos+2, data[pos+1]));
  }
}
