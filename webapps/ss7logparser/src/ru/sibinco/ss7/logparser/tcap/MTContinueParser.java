package ru.sibinco.ss7.logparser.tcap;

import java.io.PrintWriter;

/**
 * Created by: Serge Lugovoy
 * Date: 18.10.2004
 * Time: 16:40:38
 */
public class MTContinueParser extends MessageTypeParser {
  DialoguePortionParser dialoguePortionParser = new DialoguePortionParser();
  ComponentPortionParser componentPortionParser = new ComponentPortionParser();
  OrigTranscationPortionParser origTransactionPortionParser = new OrigTranscationPortionParser();
  DestTransactionPortionParser destTransactionPortionParser = new DestTransactionPortionParser();

  public MTContinueParser() {
    mtt = "CONT";
  }

  public void decode(PrintWriter pw, byte data[]) {
    super.decode(pw, data);
    int pos = 2;
    pos = origTransactionPortionParser.decode(pw, data, pos);
    pos = destTransactionPortionParser.decode(pw, data, pos);
    pos = dialoguePortionParser.decode(pw, data, pos);
    pos = componentPortionParser.decode(pw, data, pos);
  }
}
