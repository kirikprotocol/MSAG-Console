/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Apr 3, 2003
 * Time: 3:45:19 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.smsview;

import java.util.Hashtable;
import java.util.Iterator;

public class SmsDetailedRow extends SmsRow
{
  private Hashtable parameters = new Hashtable();

  public void addBodyParameter(short tag, Object value) {
    parameters.put(String.valueOf( tag ), value);
  }

  public Hashtable getBodyParameters() {
    return parameters;
  }
}
