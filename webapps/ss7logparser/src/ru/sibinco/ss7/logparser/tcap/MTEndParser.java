package ru.sibinco.ss7.logparser.tcap;

import java.io.PrintWriter;

/**
 * Created by: Serge Lugovoy
 * Date: 18.10.2004
 * Time: 16:42:10
 */
public class MTEndParser extends MessageTypeParser {
  DialoguePortionParser dialoguePortionParser = new DialoguePortionParser();
  ComponentPortionParser componentPortionParser = new ComponentPortionParser();
  DestTransactionPortionParser destTransactionPortionParser = new DestTransactionPortionParser();

  public MTEndParser() {
    mtt = "END";
  }

  public void decode(PrintWriter pw, byte data[]) {
    super.decode(pw, data);
    int pos = 2;
    pos = destTransactionPortionParser.decode(pw, data, pos);
    pos = dialoguePortionParser.decode(pw, data, pos);
    pos = componentPortionParser.decode(pw, data, pos);
  }
}
