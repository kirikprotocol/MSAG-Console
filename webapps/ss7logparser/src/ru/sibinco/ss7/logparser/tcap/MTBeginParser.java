package ru.sibinco.ss7.logparser.tcap;

import java.io.PrintWriter;

/**
 * Created by: Serge Lugovoy
 * Date: 18.10.2004
 * Time: 16:35:05
 */
public class MTBeginParser extends MessageTypeParser {
  DialoguePortionParser dialoguePortionParser = new DialoguePortionParser();
  ComponentPortionParser componentPortionParser = new ComponentPortionParser();
  OrigTranscationPortionParser origTransactionPortionParser = new OrigTranscationPortionParser();

  public MTBeginParser() {
    mtt = "BEGIN";
  }

  public void decode(PrintWriter pw, byte data[]) {
    super.decode(pw, data);
    int pos = 2;
    pos = origTransactionPortionParser.decode(pw, data, pos);
    pos = dialoguePortionParser.decode(pw, data, pos);
    pos = componentPortionParser.decode(pw, data, pos);
  }
}
