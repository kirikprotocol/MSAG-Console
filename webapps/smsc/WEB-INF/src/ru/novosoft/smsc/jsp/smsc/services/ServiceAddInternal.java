/*
 * Created by igork
 * Date: 31.10.2002
 * Time: 6:21:34
 */
package ru.novosoft.smsc.jsp.smsc.services;

import ru.novosoft.smsc.admin.Constants;
import ru.novosoft.smsc.admin.journal.SubjectTypes;
import ru.novosoft.smsc.admin.journal.Actions;
import ru.novosoft.smsc.admin.route.SME;
import ru.novosoft.smsc.jsp.SMSCErrors;

import javax.servlet.http.HttpServletRequest;
import java.util.*;

public class ServiceAddInternal extends SmeBean
{
  protected String hostName = null;

  protected String mbNext = null;
  protected String mbCancel = null;

  protected int init(List errors)
  {
    int result = super.init(errors);
    if (result != RESULT_OK)
      return result;

    if (serviceId == null) {
      serviceId = "";
      wantAlias = true;
      forceDC = true;
    }

    return RESULT_OK;
  }

  public int process(HttpServletRequest request)
  {
    int result = super.process(request);
    if (result != RESULT_OK)
      return result;

    if (mbCancel != null)
      return RESULT_DONE;
    if (mbNext != null)
      return addService();

    return RESULT_OK;
  }

  protected int addService()
  {
    if (serviceId == null || serviceId.length() == 0)
      return error(SMSCErrors.error.services.ServiceIdNotDefined);
    if (hostsManager.getSmeIds().contains(serviceId))
      return error(SMSCErrors.error.services.alreadyExists, serviceId);
    if (serviceId.length() > 15)
      return error(SMSCErrors.error.services.ServiceIdTooLong);
    if (priority < 0 || priority > Constants.MAX_PRIORITY)
      return error(SMSCErrors.error.services.invalidPriority, String.valueOf(priority));

    try {
      hostsManager.addSme(serviceId, priority, SME.SMPP, typeOfNumber, numberingPlan, convertInterfaceVersion(interfaceVersion), systemType, password, rangeOfAddress, -1, wantAlias, forceDC, timeout, receiptSchemeName, disabled, mode, proclimit, schedlimit);
      journalAppend(SubjectTypes.TYPE_service, serviceId, Actions.ACTION_ADD);
      //appContext.getStatuses().setServicesChanged(true);
    } catch (Throwable t) {
      return error(SMSCErrors.error.services.coudntAddService, t);
    }
    return RESULT_DONE;
  }

  /******************************* properties ****************************************/
  public String getMbNext()
  {
    return mbNext;
  }

  public void setMbNext(String mbNext)
  {
    this.mbNext = mbNext;
  }

  public String getMbCancel()
  {
    return mbCancel;
  }

  public void setMbCancel(String mbCancel)
  {
    this.mbCancel = mbCancel;
  }

  public String getHostName()
  {
    return hostName;
  }

  public void setHostName(String hostName)
  {
    this.hostName = hostName;
  }
}
