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

  synchronized public MaskList getMaskList() throws AdminException
  {
    MaskList maskList = null;
    try {
      maskList = new MaskList(masks);
    } catch (Exception e) {
      throw new AdminException("WSmeVisitorsFilter: failed to obtain mask list, cause: "+e.getMessage());
    }
    return maskList;
  }
  synchronized public String[] getMasks() {
    return masks;
  }
  synchronized public void setMasks(String[] masks) {
    this.masks = masks;
  }

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
