/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: May 30, 2003
 * Time: 2:38:36 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.wsme.beans;

import java.util.List;
import java.security.Principal;

public class WSmeAdsFilterFormBean extends WSmeBaseFilterFormBean
{
  private String langs[] = null;

  public int process(List errors, Principal loginedUserPrincipal)
  {
    int result = super.process(errors, loginedUserPrincipal);
    if (result != RESULT_OK) {
      mbApply = null; mbCancel = null; mbClear = null;
      return result;
    }

    if (langs == null) { // init langs
      langs = (wsmePreferences != null) ?
          wsmePreferences.getAdsFilter().getLangs() : new String[0];
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
      wsmePreferences.getAdsFilter().setLangs(langs);
    return RESULT_DONE;
  }
  private int processCancel()
  {
    langs = (wsmePreferences != null) ?
        wsmePreferences.getAdsFilter().getLangs() : new String[0];
    return RESULT_DONE;
  }
  private int processClear()
  {
    langs = new String[0];
    return RESULT_OK;
  }

  public String[] getLangs() {
    return langs;
  }
  public void setLangs(String[] langs) {
    this.langs = prepareFilterList(langs, false);
  }

}
