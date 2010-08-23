package ru.novosoft.smsc.infosme.beans.deliveries;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.region.Region;
import ru.novosoft.smsc.infosme.backend.config.InfoSmeConfig;
import ru.novosoft.smsc.infosme.backend.config.tasks.Task;
import ru.novosoft.smsc.util.Transliterator;

import javax.servlet.http.HttpServletRequest;
import java.text.SimpleDateFormat;
import java.util.Arrays;
import java.util.Iterator;

/**
 * User: artem
 * Date: 04.06.2008
 */

public class EditTaskPage extends DeliveriesPage {

  protected EditTaskPage(DeliveriesPageData pageData) {
    super(pageData);
    Task task = pageData.getTask().getTask(pageData.activeTaskRegionId);
    try {
      taskToPage(task);
    } catch (AdminException e) {
      e.printStackTrace();
    }
    pageData.activeWeekDays = new String[task.getActiveWeekDays().size()];
    int i = 0;
    for (Iterator iter = task.getActiveWeekDays().iterator(); iter.hasNext(); i++)
      pageData.activeWeekDays[i] = (String) iter.next();
    pageData.sourceAddress = task.getAddress();
  }

  private void checkAndPrepareTask(Task task) throws AdminException {
    if (pageData.name == null || (pageData.name = pageData.name.trim()).length() <= 0) {
      task.setName("");
      throw new AdminException("infosme.error.task_name_undefined");
    } else {
      if (task.getRegionId() >= 0) {
        Region r = pageData.getAppContext().getRegionsManager().getRegionById(task.getRegionId());
        task.setName(pageData.name + " (" + r.getName() + ')');
      } else
        task.setName(pageData.name + " (All Regions)");
    }

    if (task.getPriority() <= 0 || task.getPriority() > 100) {
      throw new AdminException("infosme.error.task_priority_invalid");
    }

    if (!task.isDeliveriesFileContainsTexts()) {
      if (pageData.text == null || (pageData.text = pageData.text.trim()).length() <= 0) {
        task.setText("");
        throw new AdminException("infosme.error.task_msg_text_empty");
      } else task.setText((pageData.transliterate) ? Transliterator.translit(pageData.text) : pageData.text);
    }

    InfoSmeConfig c = pageData.getInfoSmeContext().getInfoSmeConfig();

    if (c.containsTaskWithId(task.getId()))
      throw new AdminException("Task with id='" + task.getId() + "' already exists. Please specify another id");

    if (c.containsTaskWithName(task.getName()))
      throw new AdminException("Task with name='" + pageData.name + "' already exists. Please specify another name");

    if (pageData.sourceAddress == null || pageData.sourceAddress.trim().length() == 0)
      throw new AdminException("infosme.error.empty_src_addr");
    else
      task.setAddress(pageData.sourceAddress);

    if (pageData.secret && (pageData.secretText == null || pageData.secretText.trim().length() == 0))
      throw new AdminException("infosme.error.empty_secret_text");

    task.setFlash(pageData.flash);

    task.setActiveWeekDays(Arrays.asList(pageData.activeWeekDays));
    InfoSmeConfig.validate(task);

//    if (!isUserAdmin(request))
//      calculateRetryOnFail(task);
  }


  public DeliveriesPage mbNext(HttpServletRequest request) throws AdminException {
    pageToTask(pageData.getTask().getTask(pageData.oldActiveTaskRegionId));

    for (Iterator iter = pageData.getTask().tasks().iterator(); iter.hasNext();) {
      Task t = (Task) iter.next();
      if (pageData.useSameSettingsForAllRegions)
        pageToTask(t);
      checkAndPrepareTask(t);
    }

    return new GenerateDeliveriesPage(pageData);
  }

  public DeliveriesPage mbCancel(HttpServletRequest request) throws AdminException {
    return new LoadFilePage(pageData);
  }

  protected DeliveriesPage mbTest(HttpServletRequest request) throws AdminException {
    pageToTask(pageData.getTask().getTask(pageData.oldActiveTaskRegionId));
    taskToPage(pageData.getTask().getTask(pageData.activeTaskRegionId));
    if (pageData.testSmsAddress == null || pageData.testSmsAddress.trim().length() == 0)
      throw new AdminException("infosme.error.dst_addr_is_empty");

    long result = pageData.getInfoSmeContext().getInfoSme().sendSms(pageData.sourceAddress, pageData.testSmsAddress, pageData.text, pageData.flash);

    if (result != 0)
      throw new AdminException(pageData.getAppContext().getLocaleString(request.getUserPrincipal(), "infosme.error.test_sms_delivery_error") + ": " + pageData.getAppContext().getLocaleString(request.getUserPrincipal(),"smsc.errcode." + result));

    return this;
  }

  public DeliveriesPage mbUpdate(HttpServletRequest request) throws AdminException {
    pageToTask(pageData.getTask().getTask(pageData.oldActiveTaskRegionId));
    taskToPage(pageData.getTask().getTask(pageData.activeTaskRegionId));
    return this;
  }

  public int getId() {
    return EDIT_TASK_PAGE;
  }

  private void taskToPage(Task task) throws AdminException {
    pageData.retryOnFail = task.isRetryOnFail();
    pageData.deliveriesFileContainsText = task.isDeliveriesFileContainsTexts();
    Region r = pageData.getAppContext().getRegionsManager().getRegionById(task.getRegionId());
    SimpleDateFormat df = new SimpleDateFormat(DeliveriesPageData.PAGE_DATE_FORMAT);
    SimpleDateFormat tf = new SimpleDateFormat(DeliveriesPageData.PAGE_TIME_FORMAT);
    if (r != null) {
      pageData.validityPeriod = task.getValidityPeriod() == null ? "" : String.valueOf(task.getValidityPeriod());
      pageData.validityDate = task.getValidityDate() == null ? "" : df.format(r.getRegionTime(task.getValidityDate()));
      pageData.activePeriodStart = task.getActivePeriodStart() == null ? "" : tf.format(r.getRegionTime(task.getActivePeriodStart()));
      pageData.activePeriodEnd = task.getActivePeriodEnd() == null ? "" : tf.format(r.getRegionTime(task.getActivePeriodEnd()));
    } else {
      pageData.validityPeriod = task.getValidityPeriod() == null ? "" : String.valueOf(task.getValidityPeriod());
      pageData.validityDate = task.getValidityDate() == null ? "" : df.format(task.getValidityDate());
      pageData.activePeriodStart = task.getActivePeriodStart() == null ? "" : tf.format(task.getActivePeriodStart());
      pageData.activePeriodEnd = task.getActivePeriodEnd() == null ? "" : tf.format(task.getActivePeriodEnd());
    }

    if (r != null && (pageData.splitDeliveriesFile && !pageData.useSameSettingsForAllRegions)) {
      pageData.endDate = task.getEndDate() == null ? "" : df.format(r.getRegionTime(task.getEndDate()));
      pageData.startDate = task.getStartDate() == null ? "" : df.format(r.getRegionTime(task.getStartDate()));
    } else {
      pageData.endDate = task.getEndDate() == null ? "" : df.format(task.getEndDate());
      pageData.startDate = task.getStartDate() == null ? "" : df.format(task.getStartDate());
    }
    
    pageData.recondsNumber = String.valueOf(task.getActualRecordsSize());
    pageData.retryPolicy = task.getRetryPolicy();
    pageData.secret = task.isSecret();
    pageData.secretFlash = task.isSecretFlash();
    pageData.secretText = task.getSecretMessage();
    pageData.transactionMode = task.isTransactionMode();
    pageData.useDataSm = task.isUseDataSm();
    pageData.deliveryMode = task.getDeliveryMode();
    pageData.owner = pageData.getAppContext().getUserManager().getUser(task.getOwner());
  }

  private void pageToTask(Task task) throws AdminException {
    task.setRetryOnFail(pageData.retryOnFail);
    SimpleDateFormat df = new SimpleDateFormat(DeliveriesPageData.PAGE_DATE_FORMAT);
    try {
      Region r = pageData.getAppContext().getRegionsManager().getRegionById(task.getRegionId());
      if (r != null) {
        try{
          task.setEndDate(pageData.endDate.trim().length() == 0 ? null : r.getLocalTime(df.parse(pageData.endDate)));
        }catch(Exception e) {
          pageData.endDate = "";
          throw e;
        }
        try{
          task.setStartDate(pageData.startDate.trim().length() == 0 ? null : r.getLocalTime(df.parse(pageData.startDate)));
        }catch(Exception e) {
          pageData.startDate = "";
          throw e;
        }
        try{
          task.setValidityPeriod(pageData.validityPeriod.trim().length() == 0 ? null : Integer.valueOf(pageData.validityPeriod));
        }catch(Exception e) {
          pageData.validityPeriod = "";
          throw e;
        }
        try{
          task.setValidityDate(pageData.validityDate.trim().length() == 0 ? null : r.getLocalTime(df.parse(pageData.validityDate)));
        }catch(Exception e) {
          pageData.validityDate = "";
          throw e;
        }
        try{
          task.setActivePeriodStart(pageData.activePeriodStart.trim().length() == 0 ? null : r.getLocalTime(df.parse("2010.01.01 " + pageData.activePeriodStart)));
        }catch(Exception e) {
          pageData.activePeriodStart = "";
          throw e;
        }
        try{
          task.setActivePeriodEnd(pageData.activePeriodEnd.trim().length() == 0 ? null : r.getLocalTime(df.parse("2010.01.01 " + pageData.activePeriodEnd)));
        }catch(Exception e) {
          pageData.activePeriodEnd = "";
          throw e;
        }
      } else {
        try{
          task.setEndDate(pageData.endDate.trim().length() == 0 ? null : df.parse(pageData.endDate));
        }catch(Exception e) {
          pageData.endDate = "";
          throw e;
        }
        try{
          task.setStartDate(pageData.startDate.trim().length() == 0 ? null : df.parse(pageData.startDate));
        }catch(Exception e) {
          pageData.startDate = "";
          throw e;
        }
        try{
          task.setValidityPeriod(pageData.validityPeriod.trim().length() == 0 ? null : Integer.valueOf(pageData.validityPeriod));
        }catch(Exception e) {
          pageData.validityPeriod = "";
          throw e;
        }
        try{
          task.setValidityDate(pageData.validityDate.trim().length() == 0 ? null : df.parse(pageData.validityDate));
        }catch(Exception e) {
          pageData.validityDate = "";
          throw e;
        }
        try{
          task.setActivePeriodStart(pageData.activePeriodStart.trim().length() == 0 ? null : df.parse("2010.01.01 " + pageData.activePeriodStart));
        }catch(Exception e) {
          pageData.activePeriodStart = "";
          throw e;
        }
        try{
          task.setActivePeriodEnd(pageData.activePeriodEnd.trim().length() == 0 ? null : df.parse("2010.01.01 " + pageData.activePeriodEnd));
        }catch(Exception e) {
          pageData.activePeriodEnd = "";
          throw e;
        }
      }
    } catch (Throwable e) {
      e.printStackTrace();
      throw new AdminException("Invalid field format", e);
    }
    task.setDeliveryMode(pageData.deliveryMode);
    task.setRetryPolicy(pageData.retryPolicy);
    task.setSecret(pageData.secret);
    task.setSecretFlash(pageData.secretFlash);
    task.setSecretMessage(pageData.secretText);
    task.setTransactionMode(pageData.transactionMode);
    task.setUseDataSm(pageData.useDataSm);    
    task.setOwner(pageData.owner.getName());
  }

}
