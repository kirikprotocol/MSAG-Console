/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: May 13, 2003
 * Time: 5:18:40 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.wsme.beans;

import ru.novosoft.smsc.jsp.smsc.IndexBean;
import ru.novosoft.smsc.wsme.WSmeContext;
import ru.novosoft.smsc.wsme.WSme;
import ru.novosoft.smsc.admin.AdminException;

import java.util.List;

public class WSmeFormBean extends IndexBean
{
  public final static int RESULT_VISITORS = 1000;
  public final static int RESULT_LANGS = 2000;
  public final static int RESULT_ADS = 3000;

  private int menuSelection = RESULT_DONE;

  protected WSme wsme = null;

  public int process(List errors)
  {
    WSmeContext wSmeContext = WSmeContext.getInstance();
    if (wsme == null) wsme = wSmeContext.getWsme();

    int result = super.process(wSmeContext.getAppContext(),
                               errors, wSmeContext.getLoginedUserPrincipal());
    if (result != RESULT_OK)
      return result;

    if (menuSelection != RESULT_DONE)
      return menuSelection; // redirect

    //TODO: add more code here (config)

    return RESULT_OK;
  }

  public int getMenuSelection() {
    return menuSelection;
  }
  public void setMenuSelection(int menuSelection) {
    this.menuSelection = menuSelection;
  }
}
