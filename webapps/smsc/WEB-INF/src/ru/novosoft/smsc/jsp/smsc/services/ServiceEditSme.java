package ru.novosoft.smsc.jsp.smsc.services;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.Constants;
import ru.novosoft.smsc.admin.journal.SubjectTypes;
import ru.novosoft.smsc.admin.journal.Actions;
import ru.novosoft.smsc.admin.route.SME;
import ru.novosoft.smsc.jsp.SMSCErrors;

import javax.servlet.http.HttpServletRequest;
import java.util.*;

/**
 * Created by igork
 * Date: Jan 16, 2003
 * Time: 7:00:32 PM
 */
public class ServiceEditSme extends SmeBean
{

  private String mbSave = null;
  private String mbCancel = null;

  protected int init(List errors)
  {
    int result = super.init(errors);
    if (result != RESULT_OK)
      return result;

    if (serviceId == null || serviceId.length() == 0)
      return error(SMSCErrors.error.services.ServiceIdNotDefined);

    if (interfaceVersion == null) {
      SME sme = null;
      try {
        sme = appContext.getSmeManager().get(serviceId);
      } catch (AdminException e) {
        logger.error("Couldn't get sme \"" + serviceId + "\"");
        return error(SMSCErrors.error.services.ServiceNotFound, serviceId);
      }
      if (sme == null)
        return error(SMSCErrors.error.services.ServiceNotFound, serviceId);

      priority = sme.getPriority();
      systemType = sme.getSystemType();
      typeOfNumber = sme.getTypeOfNumber();
      numberingPlan = sme.getNumberingPlan();
      interfaceVersion = String.valueOf(sme.getInterfaceVersion() >> 4) + '.' + String.valueOf(sme.getInterfaceVersion() & 0xF);
      rangeOfAddress = sme.getAddrRange();
      password = sme.getPassword();
      timeout = sme.getTimeout();
      wantAlias = sme.isWantAlias();
      forceDC = sme.isForceDC();
      receiptSchemeName = sme.getReceiptSchemeName();
      disabled = sme.isDisabled();
      mode = sme.getMode();
      proclimit = sme.getProclimit();
      schedlimit = sme.getSchedlimit();
    }

    return RESULT_OK;
  }

  public int process(HttpServletRequest request)
  {
    int result = super.process(request);
    if (result != RESULT_OK)
      return result;

    if (serviceId == null) {
      serviceId = "";
      wantAlias = true;
    }

    if (mbCancel != null)
      return RESULT_DONE;
    if (mbSave != null)
      return save();

    return RESULT_OK;
  }

  private int save()
  {
    if (serviceId == null || serviceId.length() == 0)
      return error(SMSCErrors.error.services.ServiceIdNotDefined);
    if (!hostsManager.getSmeIds().contains(serviceId))
      return error(SMSCErrors.error.services.ServiceIdNotDefined, serviceId);
    if (serviceId.length() > 15)
      return error(SMSCErrors.error.services.ServiceIdTooLong);
    if (priority < 0 || priority > Constants.MAX_PRIORITY)
      return error(SMSCErrors.error.services.invalidPriority, String.valueOf(priority));

    try {
      SME sme = new SME(serviceId, priority, SME.SMPP, typeOfNumber, numberingPlan, convertInterfaceVersion(interfaceVersion), systemType, password, rangeOfAddress, -1, wantAlias, forceDC, timeout, receiptSchemeName, disabled, mode, proclimit, schedlimit);
      if (hostsManager.isService(serviceId))
        hostsManager.getServiceInfo(serviceId).setSme(sme);
      appContext.getSmeManager().update(sme);
      journalAppend(SubjectTypes.TYPE_service, serviceId, Actions.ACTION_MODIFY);
      //appContext.getStatuses().setServicesChanged(true);
    } catch (Throwable t) {
      logger.error("Couldn't update SME parameters", t);
      return error(SMSCErrors.error.services.coudntAddService, t);
    }
    return RESULT_DONE;
  }

  public String getMbSave()
  {
    return mbSave;
  }

  public void setMbSave(String mbSave)
  {
    this.mbSave = mbSave;
  }

  public String getMbCancel()
  {
    return mbCancel;
  }

  public void setMbCancel(String mbCancel)
  {
    this.mbCancel = mbCancel;
  }

}
