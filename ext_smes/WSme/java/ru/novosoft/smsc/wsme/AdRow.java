/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: May 14, 2003
 * Time: 1:49:59 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.wsme;

public class AdRow
{
  public int    id;
  public String lang;
  public String ad;

  public AdRow(int id, String lang, String ad)
  {
    this.id = id;
    this.lang = (lang != null) ? lang.trim():null;
    this.ad = (ad != null) ? ad.trim():null;
  }
}
