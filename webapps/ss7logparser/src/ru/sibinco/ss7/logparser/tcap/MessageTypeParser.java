package ru.sibinco.ss7.logparser.tcap;

import java.io.PrintWriter;

/**
 * Created by: Serge Lugovoy
 * Date: 18.10.2004
 * Time: 16:09:40
 */
public class MessageTypeParser {
  String mtt = "";
  public void decode( PrintWriter pw, byte data[] ) {
    pw.print(mtt);
    pw.print(": ");
  }
}
