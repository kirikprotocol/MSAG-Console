package ru.sibinco.ss7.logparser.tcap;

import ru.sibinco.ss7.logparser.Util;

import java.io.PrintWriter;

/**
 * Created by: Serge Lugovoy
 * Date: 18.10.2004
 * Time: 16:15:37
 */
public class DialoguePortionParser {
  public int decode( PrintWriter pw, byte data[], int pos ) {
    pw.print("(dlg: ");
    pw.print(Util.hex(data[pos++]));
    pw.print(' ');
    int len = Util.unsigned(data[pos++]);
    pw.print(len);
    pw.print(' ');
    pw.print(Util.hex(data,pos,len));
    pw.print(')');
    return pos+len;
  }
}
