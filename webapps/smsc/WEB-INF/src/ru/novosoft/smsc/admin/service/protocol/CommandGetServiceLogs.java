/*
 * Created by igork
 * Date: Mar 19, 2002
 * Time: 11:07:53 PM
 */
package ru.novosoft.smsc.admin.protocol;

import org.w3c.dom.Element;

public class CommandGetServiceLogs extends Command
{
  public CommandGetServiceLogs(long startpos, long length)
  {
    super("get_logs");

    Element logsElem = document.createElement("logs");
    logsElem.setAttribute("start", Long.toString(startpos));
    logsElem.setAttribute("length", Long.toString(length));
    document.getDocumentElement().appendChild(logsElem);
  }
}
