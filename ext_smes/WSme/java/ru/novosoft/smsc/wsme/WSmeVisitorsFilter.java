/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: May 30, 2003
 * Time: 3:45:45 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.wsme;

import ru.novosoft.smsc.admin.route.MaskList;
import ru.novosoft.smsc.admin.AdminException;

public class WSmeVisitorsFilter
{
  private String[] masks = {};

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
}
