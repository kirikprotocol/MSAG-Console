/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: May 30, 2003
 * Time: 1:49:07 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.wsme.beans;

import java.util.List;
import java.security.Principal;

public class WSmeVisitorsFilterFormBean extends WSmeBaseFilterFormBean
{
  private String masks[] = null;

  public int process(List errors, Principal loginedUserPrincipal)
  {
    int result = super.process(errors, loginedUserPrincipal);
    if (result != RESULT_OK) {
      mbApply = null; mbCancel = null; mbClear = null;
      return result;
    }

    if (masks == null) { // init masks
      masks = (wsmePreferences != null) ?
          wsmePreferences.getVisitorsFilter().getMasks() : new String[0];
    }

    if (mbApply != null)        result = processApply();
    else if (mbCancel != null)  result = processCancel();
    else if (mbClear != null)   result = processClear();

    mbApply = null; mbCancel = null; mbClear = null;
    return result;
  }
  private int processApply()
  {
    if (wsmePreferences != null)
      wsmePreferences.getVisitorsFilter().setMasks(masks);
    return RESULT_DONE;
  }
  private int processCancel()
  {
    masks = (wsmePreferences != null) ?
        wsmePreferences.getVisitorsFilter().getMasks() : new String[0];
    return RESULT_DONE;
  }
  private int processClear()
  {
    masks = new String[0];
    return RESULT_OK;
  }

  public String[] getMasks() {
    return masks;
  }
  public void setMasks(String[] masks) {
    this.masks = prepareFilterList(masks, true);
  }

}
