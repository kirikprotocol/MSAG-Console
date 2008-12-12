package ru.novosoft.smsc.infosme.beans.deliveries;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.infosme.backend.Task;
import ru.novosoft.smsc.util.Transliterator;

import javax.servlet.http.HttpServletRequest;
import java.util.Iterator;
import java.util.Calendar;
import java.util.Arrays;
import java.text.ParseException;

/**
 * User: artem
 * Date: 04.06.2008
 */

public class EditTaskPage extends DeliveriesPage {

  protected EditTaskPage(DeliveriesPageData pageData) {
    super(pageData);
    Task task = pageData.getTask().getTask(pageData.activeTaskSubject);
    taskToPage(task);
    pageData.activeWeekDays = new String[task.getActiveWeekDays().size()];
    int i=0;
    for (Iterator iter = task.getActiveWeekDays().iterator(); iter.hasNext(); i++)
      pageData.activeWeekDays[i] = (String)iter.next();
    pageData.sourceAddress = task.getAddress();
  }

  private long calculateTotalTime(Task task)  {
    long result = 0;
    String[] days = new String[] {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    final long endDate = task.getEndDateDate().getTime();

    final Calendar start = Calendar.getInstance();
    final Calendar end = Calendar.getInstance();

    start.setTimeInMillis(task.getActivePeriodStartDate().getTime());
    end.setTimeInMillis(task.getActivePeriodEndDate().getTime());

    final long dayCount = end.getTimeInMillis() - start.getTimeInMillis();

    final Calendar cal = Calendar.getInstance();

    long date = System.currentTimeMillis();
    cal.setTimeInMillis(date);

    start.set(Calendar.DAY_OF_MONTH, cal.get(Calendar.DAY_OF_MONTH));
    start.set(Calendar.MONTH, cal.get(Calendar.MONTH));
    start.set(Calendar.YEAR, cal.get(Calendar.YEAR));

    end.set(Calendar.DAY_OF_MONTH, cal.get(Calendar.DAY_OF_MONTH));
    end.set(Calendar.MONTH, cal.get(Calendar.MONTH));
    end.set(Calendar.YEAR, cal.get(Calendar.YEAR));

    if (cal.getTime().after(start.getTime())) {
      if (task.isWeekDayActive(days[cal.get(Calendar.DAY_OF_WEEK) - 1]))
        result += (end.getTimeInMillis() - cal.getTimeInMillis());
      date += 1000 * 60 * 60 * 24;
    }

    for (; date < endDate; date += 1000 * 60 * 60 * 24) {
      cal.setTimeInMillis(date);
      if (task.isWeekDayActive(days[cal.get(Calendar.DAY_OF_WEEK) - 1]))
        result += dayCount;
    }

    return result;
  }

  private String getDoubleCharNumber(long num) {
    String res = String.valueOf(num);
    if (res.length() == 1)
      res = '0' + res;
    return res;
  }

  private void calculateRetryOnFail(Task task) throws AdminException {
    long retryOnFail = -1;
    if (task.getEndDate() == null || task.getEndDate().length() == 0)
      retryOnFail = 24*3600;
    else {
      try {
        final int maxSmsSec = pageData.getInfoSmeContext().getConfig().getInt("InfoSme.maxMessagesPerSecond") >> 1;
        final long totalTime = calculateTotalTime(task) / 1000 ; // total time in seconds

        long trySeconds = task.getActualRecordsSize() / maxSmsSec ; // total work time in seconds
        if (trySeconds == 0)
          trySeconds = 1; // One second will be enough

        long ntries = totalTime / trySeconds; // Count of tries

        if (ntries - 1 > 0)
          retryOnFail = ntries * trySeconds;

      } catch (Exception e) {
        throw new AdminException(e.getMessage());
      }
    }

    if (retryOnFail > 0) {
      task.setRetryOnFail(true);
      long hours = Math.min(retryOnFail/3600, 24);
      long minutes = hours == 24 ? 0 : (retryOnFail - hours*3600) / 60;
      long seconds = hours == 24 ? 0 : retryOnFail - hours*3600 - minutes*60;
      task.setRetryTime(getDoubleCharNumber(hours) + ':' + getDoubleCharNumber(minutes) + ':' + getDoubleCharNumber(seconds));
    } else
      task.setRetryOnFail(false);
  }

  private static boolean isUserAdmin(HttpServletRequest request) {
    return request.isUserInRole("infosme-admin");
  }

  private void checkAndPrepareTask(Task task, HttpServletRequest request) throws AdminException {
    if (pageData.name == null || (pageData.name = pageData.name.trim()).length() <= 0) {
      task.setName("");
      throw new AdminException("infosme.error.task_name_undefined");
    } else
      task.setName(pageData.name + " (" + task.getSubject() + ')');

    if (task.getPriority() <= 0 || task.getPriority() > 100) {
      throw new AdminException("infosme.error.task_priority_invalid");
    }

    if (pageData.text == null || (pageData.text = pageData.text.trim()).length() <= 0) {
      task.setText(""); throw new AdminException("infosme.error.task_msg_text_empty");
    }
    else task.setText((pageData.transliterate) ? Transliterator.translit(pageData.text):pageData.text);

    if (task.isContainsInConfig(pageData.getInfoSmeContext().getConfig()))
      throw new AdminException("Task with id='" + task.getId() + "' already exists. Please specify another id");

    if (task.isContainsInConfigByName(pageData.getInfoSmeContext().getConfig()))
      throw new AdminException("Task with name='" + pageData.name + "' already exists. Please specify another name");

    if (pageData.sourceAddress == null || pageData.sourceAddress.trim().length() == 0)
      throw new AdminException("infosme.error.empty_src_addr");
    else
      task.setAddress(pageData.sourceAddress);

    if (pageData.secret && (pageData.secretText == null || pageData.secretText.trim().length() == 0))
      throw new AdminException("infosme.error.empty_secret_text");

    task.setFlash(pageData.flash);

    task.setActiveWeekDays(Arrays.asList(pageData.activeWeekDays));        

    if (!isUserAdmin(request))
      calculateRetryOnFail(task);
  }

  public DeliveriesPage mbNext(HttpServletRequest request) throws AdminException {
    pageToTask(pageData.getTask().getTask(pageData.oldActiveTaskSubject));

    for (Iterator iter = pageData.getTask().tasks().iterator(); iter.hasNext();)
      checkAndPrepareTask((Task)iter.next(), request);

    return new GenerateDeliveriesPage(pageData);
  }

  public DeliveriesPage mbCancel(HttpServletRequest request) throws AdminException {
    return new StartPage(pageData);
  }

  public DeliveriesPage mbUpdate(HttpServletRequest request) throws AdminException {
    pageToTask(pageData.getTask().getTask(pageData.oldActiveTaskSubject));
    taskToPage(pageData.getTask().getTask(pageData.activeTaskSubject));
    return this;
  }

  public int getId() {
    return EDIT_TASK_PAGE;
  }

  private void taskToPage(Task task) {
    pageData.retryOnFail = task.isRetryOnFail();
    pageData.retryTime = task.getRetryTime();
    pageData.endDate = task.getEndDate();
    pageData.startDate = task.getStartDate();
    pageData.validityPeriod = task.getValidityPeriod();
    pageData.validityDate = task.getValidityDate();
    pageData.activePeriodStart = task.getActivePeriodStart();
    pageData.activePeriodEnd = task.getActivePeriodEnd();
    pageData.recondsNumber = String.valueOf(task.getActualRecordsSize());
    pageData.retryPolicy = task.getRetryPolicy();
    pageData.secret = task.isSecret();
    pageData.secretFlash = task.isSecretFlash();
    pageData.secretText = task.getSecretMessage();
    pageData.transactionMode = task.isTransactionMode();
  }

  private void pageToTask(Task task) {
    task.setRetryOnFail(pageData.retryOnFail);
    task.setRetryTime(pageData.retryTime);
    task.setEndDate(pageData.endDate);
    task.setStartDate(pageData.startDate);
    task.setValidityPeriod(pageData.validityPeriod);
    task.setValidityDate(pageData.validityDate);
    task.setActivePeriodStart(pageData.activePeriodStart);
    task.setActivePeriodEnd(pageData.activePeriodEnd);
    task.setRetryPolicy(pageData.retryPolicy);
    task.setSecret(pageData.secret);
    task.setSecretFlash(pageData.secretFlash);
    task.setSecretMessage(pageData.secretText);
    task.setTransactionMode(pageData.transactionMode);
  }

}
