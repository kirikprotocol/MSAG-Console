package ru.sibinco.ss7.logparser.tcap;

import ru.sibinco.ss7.logparser.Util;

import java.io.PrintWriter;

/**
 * Created by: Serge Lugovoy
 * Date: 18.10.2004
 * Time: 16:43:11
 */
public class MTPAbortParser extends MessageTypeParser {
  String causes[] = new String [] {
"Unrecognized Message Type",
"Unrecognized Transaction ID",
"Badly Formatted Transaction Portion",
"Incorrect Transaction Portion",
"Resource Limitations",
"Abnormal Dialogue"
  };

  DestTransactionPortionParser destTransactionPortionParser = new DestTransactionPortionParser();

  public MTPAbortParser() {
    mtt = "PABORT";
  }

  public void decode(PrintWriter pw, byte data[]) {
    super.decode(pw, data);
    int pos = 2;
    pos = destTransactionPortionParser.decode(pw, data, pos);
    int cause = Util.unsigned(data[pos+2]);
    if( cause < causes.length ) pw.print(" "+causes[cause]);
    else pw.print(" cause: "+cause);
  }

}
