/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: May 30, 2003
 * Time: 4:23:08 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.wsme.beans;

import ru.novosoft.smsc.admin.route.Mask;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.jsp.PageBean;
import ru.novosoft.smsc.wsme.WSmeContext;
import ru.novosoft.smsc.wsme.WSmePreferences;
import ru.novosoft.smsc.util.Functions;

import javax.servlet.http.HttpServletRequest;
import java.util.*;

public class WSmeBaseFilterFormBean extends PageBean
{
  protected WSmePreferences wsmePreferences = null;

  protected String mbApply = null;
  protected String mbClear = null;
  protected String mbCancel = null;

  protected String[] prepareFilterList(String[] list, boolean mask)
  {
    if (list == null) return null;

    List result = new ArrayList();
    for (int i = 0; i < list.length; i++) {
      String value = (list[i] != null) ? list[i].trim() : null;
      if (value != null && value.length() > 0) {
        try {
          if (mask)
            result.add((new Mask(value)).getMask());
          else
            result.add(value);
        } catch (Exception e) {
          System.out.println("WSmeBaseFilter: " + e.getMessage());
        }
      }
    }
    String cleaned[] = new String[result.size()];
    for (int i = 0; i < result.size(); i++)
      cleaned[i] = (String) result.get(i);
    return cleaned;
  }

  public int process(HttpServletRequest request)
  {
    String smeId = "WSme";
    try {
      smeId = Functions.getServiceId(request.getServletPath());
    } catch (AdminException e) {
      logger.error("Could not discover service id", e);
      error("Could not discover service id, WSme assumed", e);
    }
    WSmeContext wSmeContext = WSmeContext.getInstance(Functions.getAppContext(request), smeId);

    int result = super.process(request);
    if (result != RESULT_OK)
      return result;

    if (wsmePreferences == null)
      wsmePreferences = wSmeContext.getWSmePreferences(request.getUserPrincipal());

    return RESULT_OK;
  }

  public String getMbApply()
  {
    return mbApply;
  }

  public void setMbApply(String mbApply)
  {
    this.mbApply = mbApply;
  }

  public String getMbCancel()
  {
    return mbCancel;
  }

  public void setMbCancel(String mbCancel)
  {
    this.mbCancel = mbCancel;
  }

  public String getMbClear()
  {
    return mbClear;
  }

  public void setMbClear(String mbClear)
  {
    this.mbClear = mbClear;
  }

}
