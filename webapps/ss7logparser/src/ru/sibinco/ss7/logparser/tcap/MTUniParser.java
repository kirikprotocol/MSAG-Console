package ru.sibinco.ss7.logparser.tcap;

import java.io.PrintWriter;

/**
 * Created by: Serge Lugovoy
 * Date: 18.10.2004
 * Time: 16:14:50
 */
public class MTUniParser extends MessageTypeParser{
  DialoguePortionParser dialoguePortionParser = new DialoguePortionParser();
  ComponentPortionParser componentPortionParser = new ComponentPortionParser();

  public MTUniParser() {
    mtt = "UNI";
  }

  public void decode(PrintWriter pw, byte data[]) {
    super.decode(pw, data);
    int pos = 2;
    pos = dialoguePortionParser.decode(pw, data, pos);
    pos = componentPortionParser.decode(pw, data, pos);
  }
}
