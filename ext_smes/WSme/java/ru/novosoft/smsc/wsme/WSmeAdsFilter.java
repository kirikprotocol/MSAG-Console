/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: May 30, 2003
 * Time: 3:46:55 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.wsme;

import java.util.List;
import java.util.ArrayList;

public class WSmeAdsFilter
{
  private String[] langs = {};

  synchronized public List getLangList()
  {
    List langList = new ArrayList();
    for (int i=0; i<langs.length; i++)
      langList.add(langs[i]);
    return langList;
  }
  synchronized public String[] getLangs() {
    return langs;
  }
  synchronized public void setLangs(String[] langs) {
    this.langs = langs;
  }
}
