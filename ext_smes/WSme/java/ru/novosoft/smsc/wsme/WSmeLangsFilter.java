/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: May 30, 2003
 * Time: 3:46:40 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.wsme;

import ru.novosoft.smsc.admin.route.MaskList;
import ru.novosoft.smsc.admin.AdminException;

import java.util.List;
import java.util.ArrayList;

public class WSmeLangsFilter
{
  private String[] masks = {};
  private String[] langs = {};

  public MaskList getMaskList() throws AdminException
  {
    synchronized(masks) {
      MaskList maskList = null;
      try {
        maskList = new MaskList(masks);
      } catch (Exception e) {
        throw new AdminException("WSmeVisitorsFilter: failed to obtain mask list, cause: "+e.getMessage());
      }
      return maskList;
    }
  }
  public String[] getMasks() {
    synchronized(masks) {
      return masks;
    }
  }
  public void setMasks(String[] masks) {
    synchronized(masks) {
      this.masks = masks;
    }
  }

  public List getLangList()
  {
    synchronized(langs) {
      List langList = new ArrayList();
      for (int i=0; i<langs.length; i++)
        langList.add(langs[i]);
      return langList;
    }
  }
  public String[] getLangs() {
    synchronized(langs) {
      return langs;
    }
  }
  public void setLangs(String[] langs) {
    synchronized(langs) {
      this.langs = langs;
    }
  }
}
