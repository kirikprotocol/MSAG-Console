/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: May 14, 2003
 * Time: 1:46:28 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.wsme;

public class LangRow
{
  public String mask;
  public String lang;

  public LangRow(String mask, String lang)
  {
    this.mask = (mask != null) ? mask.trim():null;
    this.lang = (lang != null) ? lang.trim():null;
  }
}
