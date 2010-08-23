package ru.novosoft.smsc.infosme.beans.deliveries;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.region.Region;
import ru.novosoft.smsc.admin.profiler.SupportExtProfile;
import ru.novosoft.smsc.infosme.backend.config.tasks.Task;
import ru.novosoft.smsc.infosme.backend.deliveries.DeliveriesGenerationThread;
import ru.novosoft.smsc.infosme.beans.InfoSmeBean;

import javax.servlet.http.HttpServletRequest;
import java.util.Collection;
import java.util.List;
import java.util.Map;

/**
 * User: artem
 * Date: 13.05.2008
 */

public class Deliveries extends InfoSmeBean {

  private DeliveriesPage activePage;
  private DeliveriesPageData pageData;

  // Page data
  private String mbNext;
  private String mbCancel;
  private String mbTest;

  protected int init(List errors) {
    int result = super.init(errors);
    if (result != RESULT_OK)
      return result;

    if (this.activePage == null) {
      pageData = new DeliveriesPageData(appContext, getInfoSmeContext());
      this.activePage = new LoadFilePage(pageData);
    }
    
    return result;
  }

  public int process(HttpServletRequest request) {
    int result = super.process(request);
    if (result != RESULT_OK)
      return result;

    pageData.owner = pageData.getAppContext().getUserManager().getUser(request.getRemoteUser());
    
    setTransactionMode(request.getParameter("transactionMode") != null );
    setUseDataSm(request.getParameter("useDataSm") != null) ;    

    try {
      if (mbNext != null) {
        mbNext = null;
        activePage = activePage.mbNext(request);
      } else if (mbCancel != null) {
        mbCancel = null;
        activePage = activePage.mbCancel(request);
      } else if (mbTest != null) {
        mbTest = null;
        activePage = activePage.mbTest(request);
      } else
        activePage = activePage.mbUpdate(request);
    } catch (AdminException e) {
      logger.error(e,e);
      return error(e.getMessage());
    }

    return result;
  }

  public int getActiveTaskRegionId() {
    return pageData.activeTaskRegionId;
  }

  public void setActiveTaskRegionId(int id) {
    this.pageData.activeTaskRegionId = id;
  }

  public Collection getRegionIds() {
    return pageData.getTask().getRegionIds();
  }

  public int getActualSize(int regionId) {
    Task t = pageData.getTask().getTask(regionId);
    return t == null ? 0 :t.getActualRecordsSize();
  }    

  public String getRegionName(int id) {
    try {
      if (id == -1)
        return "All regions";
      Region r = pageData.getAppContext().getRegionsManager().getRegionById(id);
      return r.getName();
    } catch (Exception e) {
      logger.error(e,e);
      return null;
    }
  }

  public String getName() {
    return pageData.name;
  }

  public void setName(String name) {
    this.pageData.name = name;
  }

  public boolean isTransliterate() {
    return pageData.transliterate;
  }
  public void setTransliterate(boolean transliterate) {
    this.pageData.transliterate = transliterate;
  }

  public boolean isTransactionMode() {
    return pageData.transactionMode;
  }

  public boolean isFileContainsText() {
    return this.pageData.deliveriesFileContainsText;
  }

  public void setTransactionMode(boolean val) {
    this.pageData.transactionMode = val;
  }

  public boolean isRetryOnFail() {
    return pageData.retryOnFail;
  }

  public void setRetryOnFail(boolean retryOnFail) {
    this.pageData.retryOnFail = retryOnFail;
  }

  public String getEndDate() {
    return pageData.endDate;
  }

  public void setEndDate(String endDate) {
    this.pageData.endDate = endDate;
  }

  public String getStartDate() {
    return pageData.startDate;
  }

  public void setStartDate(String startDate) {
    this.pageData.startDate = startDate;
  }

  public String getValidityPeriod() {
    return pageData.validityPeriod;
  }

  public void setValidityPeriod(String validityPeriod) {
    this.pageData.validityPeriod = validityPeriod;
  }

  public String getValidityDate() {
    return pageData.validityDate;
  }
  public void setValidityDate(String validityDate) {
    this.pageData.validityDate = validityDate;
  }

  public String getActivePeriodStart() {
    return pageData.activePeriodStart;
  }
  public void setActivePeriodStart(String activePeriodStart) {
    this.pageData.activePeriodStart = activePeriodStart;
  }

  public String getActivePeriodEnd() {
    return pageData.activePeriodEnd;
  }
  public void setActivePeriodEnd(String activePeriodEnd) {
    this.pageData.activePeriodEnd = activePeriodEnd;
  }

  public String[] getActiveWeekDays() {
    return this.pageData.activeWeekDays;
  }

  public void setActiveWeekDays(String[] activeWeekDays) {
    this.pageData.activeWeekDays = new String[activeWeekDays.length];
    System.arraycopy(activeWeekDays, 0, this.pageData.activeWeekDays, 0, activeWeekDays.length);
  }

  public boolean isWeekDayActive(String weekday) {
    for (int i=0; i<pageData.activeWeekDays.length; i++) {
      if (pageData.activeWeekDays[i] != null && pageData.activeWeekDays[i].equalsIgnoreCase(weekday))
        return true;
    }
    return false;
  }



  public String getActiveWeekDaysString() {
    StringBuffer str = new StringBuffer(30);
    int total = (pageData.activeWeekDays == null) ? 0 : pageData.activeWeekDays.length;
    if (total > 0) {
      int added=0;
      if (isWeekDayActive("Mon")) { str.append("Monday");    if (++added < total) str.append(", "); }
      if (isWeekDayActive("Tue")) { str.append("Tuesday");   if (++added < total) str.append(", "); }
      if (isWeekDayActive("Wed")) { str.append("Wednesday"); if (++added < total) str.append(", "); }
      if (isWeekDayActive("Thu")) { str.append("Thursday");  if (++added < total) str.append(", "); }
      if (isWeekDayActive("Fri")) { str.append("Friday");    if (++added < total) str.append(", "); }
      if (isWeekDayActive("Sat")) { str.append("Saturday");  if (++added < total) str.append(", "); }
      if (isWeekDayActive("Sun"))   str.append("Sunday");
    }
    return str.toString();
  }

  public String getText() {
    return pageData.text;
  }

  public void setText(String text) {
    this.pageData.text = text;
  }

  public String getSecretText() {
    return pageData.secretText;
  }

  public void setSecretText(String text) {
    pageData.secretText = text;
  }

  public boolean isSecretFlash() {
    return pageData.secretFlash;
  }

  public void setSecretFlash(boolean val) {
    pageData.secretFlash = val;
  }

  public String getTestSmsAddress() {
    return pageData.testSmsAddress == null ? "" : pageData.testSmsAddress;
  }

  public void setTestSmsAddress(String address) {
    pageData.testSmsAddress = address;
  }

  public Map getGenerationProgress() {
    return pageData.deliveriesGenProgr;
  }

  public String getMbNext() {
    return mbNext;
  }

  public void setMbNext(String mbNext) {
    this.mbNext = mbNext;
  }

  public String getMbCancel() {
    return mbCancel;
  }

  public void setMbCancel(String mbCancel) {
    this.mbCancel = mbCancel;
  }

  public String getMbTest() {
    return mbTest;
  }

  public void setMbTest(String mbTest) {
    this.mbTest = mbTest;
  }

  public String getStatusStr() {
    return pageData.getDeliveriesGenStatus();
  }

  public String getProcessFileStatusStr() {
    return pageData.getProcessFileStatus();
  }

  public int getActivePageId() {
    return activePage.getId();
  }

  public boolean isFlash() {
    return pageData.flash;
  }

  public void setFlash(boolean flash) {
    this.pageData.flash = flash;
  }

  public boolean isSecret() {
    return pageData.secret;
  }

  public void setSecret(boolean val) {
    pageData.secret = val;
  }

  public boolean isSupportExtProfile() {
    try {
      return SupportExtProfile.enabled && appContext.getSmsXSender() != null;
    } catch (AdminException e) {
      logger.error(e,e);
      return false;
    }
  }

  public String getSourceAddress() {
    return pageData.sourceAddress;
  }

  public void setSourceAddress(String sourceAddress) {
    this.pageData.sourceAddress = sourceAddress;
  }

  public int getOldActiveTaskRegionId() {
    return pageData.oldActiveTaskRegionId;
  }

  public void setOldActiveTaskRegionId(int oldActiveTaskRegionId) {
    this.pageData.oldActiveTaskRegionId = oldActiveTaskRegionId;
  }

  public int getMessages(int regionId) {
    if (pageData.deliveriesGenProgr == null || pageData.deliveriesGenProgr.get(new Integer(regionId)) == null)
      return 0;
    return ((DeliveriesGenerationThread.Progress)pageData.deliveriesGenProgr.get(new Integer(regionId))).getMsgCount();
  }

  public String getRecordsNumber() {
    return pageData.recondsNumber;
  }

  public double getProgress(int regionId) {
    if (pageData.deliveriesGenProgr == null || pageData.deliveriesGenProgr.get(new Integer(regionId)) == null)
      return 0;
    return ((DeliveriesGenerationThread.Progress)pageData.deliveriesGenProgr.get(new Integer(regionId))).getProgressPercent();
  }

  public int getGenerationStatus() {
    return pageData.deliveriesGenStatus;
  }

  public int getRecordsProcessed() {
    return pageData.recordsProcessed;
  }

  public int getUnrecognized() {
    return pageData.unrecognized;
  }

  public int getInBlackList() {
    return pageData.inblackList;
  }

  public int getRegionsFound() {
    return pageData.regionsFound;
  }

  public boolean isSplitDeliveriesFile() {
    return pageData.splitDeliveriesFile;
  }

  public void setSplitDeliveriesFile(boolean val) {
    this.pageData.splitDeliveriesFile = val;
  }

  public boolean isUseSameSettingsForAllRegions() {
    return this.pageData.useSameSettingsForAllRegions;
  }

  public void setUseSameSettingsForAllRegions(boolean value) {
    this.pageData.useSameSettingsForAllRegions = value;
  }

  public List getRetryPolicies() throws AdminException {
    return pageData.getRetryPolicies();
  }

  public String getRetryPolicy() {
    return pageData.retryPolicy;
  }

  public void setRetryPolicy(String policy) {
    pageData.retryPolicy = policy;
  }

  public boolean isUseDataSm() {
    return pageData.useDataSm;
  }

  public void setUseDataSm(boolean val) {
    pageData.useDataSm = val; 
  }

    public boolean isUssdPushAllowed() {
        return getInfoSmeConfig().hasUssdPushFeature();
    }

  public int getDeliveryMode() {
    return pageData.deliveryMode;
  }

  public void setDeliveryMode(int mode) {
    pageData.deliveryMode = mode;
    if(mode != Task.DELIVERY_MODE_SMS) {
      pageData.useDataSm = false;
      pageData.transactionMode = true;
      pageData.flash = false;
    }
  }


}
