package ru.sibinco.ss7.logparser.tcap;

import ru.sibinco.ss7.logparser.Util;

import java.io.PrintWriter;

/**
 * Created by: Serge Lugovoy
 * Date: 18.10.2004
 * Time: 16:57:28
 */
public class MTAbortParser extends MessageTypeParser {
  MTUAbortParser uAbortParser = new MTUAbortParser();
  MTPAbortParser pAbortParser = new MTPAbortParser();

  public MTAbortParser() {
    mtt = "ABORT";
  }

  public void decode(PrintWriter pw, byte data[]) {
    if( data[2+2+data[3]] == 0x4A ) {
      pAbortParser.decode(pw, data);
    } else if( data[2+2+data[3]] == 0x8B ) {
      uAbortParser.decode(pw, data);
    } else {
      super.decode(pw, data);
      pw.print("Unknown abort packet");
    }
  }
}
