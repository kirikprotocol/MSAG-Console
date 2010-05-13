package ru.novosoft.smsc.infosme.backend.commands;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.region.Region;
import ru.novosoft.smsc.admin.route.Subject;
import ru.novosoft.smsc.admin.users.User;
import ru.novosoft.smsc.infosme.backend.BlackListManager;
import ru.novosoft.smsc.infosme.backend.InfoSmeContext;
import ru.novosoft.smsc.infosme.backend.Message;
import ru.novosoft.smsc.infosme.backend.config.tasks.Task;
import ru.novosoft.smsc.infosme.backend.radixtree.TemplatesRadixTree;
import ru.novosoft.smsc.infosme.beans.InfoSmeBean;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.util.Functions;
import ru.novosoft.smsc.util.config.Config;

import java.io.*;
import java.util.*;

import org.apache.log4j.Category;
import org.xml.sax.SAXException;

import javax.xml.parsers.ParserConfigurationException;

/**
 * User: artem
 * Date: Aug 6, 2007
 */

public class InfoSmeTaskManager implements Runnable {

  private static final Category category = Category.getInstance(InfoSmeTaskManager.class);

  private final LinkedList distributions = new LinkedList();

  private final static String SME_ID = "InfoSme";

  private final SMSCAppContext appContext;

  public InfoSmeTaskManager(SMSCAppContext appContext) {
    this.appContext = appContext;
  }

  private boolean started = false;

  public boolean addTask(Task task, User user, String filename, boolean splitByRegions) {
    if (!started) {
      return false;
    }
    if (task == null || user == null || filename == null) {
      throw new IllegalArgumentException("Some arguments are null");
    }
    Distribution d = new Distribution(task, user, filename);
    d.splitByRegions = splitByRegions;
    synchronized (lock) {
      distributions.addLast(d);
    }
    return true;
  }


  private final Object lock = new Object();

  private final Object shutdownLock = new Object();

  /**
   * @noinspection EmptyCatchBlock
   */
  public void run() {
    started = true;
    if (category.isDebugEnabled()) {
      category.debug("InfoSmeTaskManager is started");
    }
    while (started) {
      try {
        Distribution d = null;
        synchronized (lock) {
          if (!distributions.isEmpty()) {
            d = (Distribution) distributions.removeFirst();
          }
        }
        if (d != null) {
          synchronized (shutdownLock) {
            handle(d);
          }
        } else {
          try {
            Thread.sleep(5000);
          } catch (InterruptedException e) {
          }
        }
      } catch (Throwable e) {
        category.error(e, e);
      }
    }
  }

  public void start() {
    if (!started) {
      new Thread(this, "InfoSmeTaskManager").start();
    }
  }

  public void shutdown() {
    started = false;
    synchronized (shutdownLock) {
      if (category.isDebugEnabled()) {
        category.debug("InfoSmeTaskManager is shutdowned");
      }
    }
  }

  private static Message readMessage(String line) {
    int index = line.indexOf('|');
    if (index < 0)
      return null;
    String msisdn = line.substring(0, index);
    if (msisdn.charAt(0) != '+')
      msisdn = '+' + msisdn;

    String text = line.substring(index + 1);
    text = text.replaceAll("\\\\r", "").replaceAll("\\\\n", System.getProperty("line.separator"));

    final Message msg = new Message();
    msg.setAbonent(msisdn);
    msg.setMessage(text);
    msg.setState(Message.State.NEW);

    return msg;
  }

  private void handle(Distribution d) throws Config.WrongParamTypeException, IOException, ParserConfigurationException,
      AdminException, Config.ParamNotFoundException, SAXException {

    InfoSmeContext smeContext = InfoSmeContext.getInstance(appContext, SME_ID);
    if (smeContext == null)
      throw new AdminException("Infosme is offline");

    BlackListManager blm = smeContext.getBlackListManager();

    String fileName = d.filename;
    User user = d.user;
    Task task = d.task;

    File file = new File(fileName);
    if (!file.exists()) {
      category.warn("File to process doesn't exist: " + fileName);
      return;
    }

    if (category.isDebugEnabled()) 
      category.debug("Start task's generation for: " + fileName);

    TemplatesRadixTree rtree = new TemplatesRadixTree();
    initiateRadixTree(rtree, user);

    Map tasks = new HashMap();

    BufferedReader is = null;
    try {
      is = new BufferedReader(new InputStreamReader(new FileInputStream(file), Functions.getLocaleEncoding()));

      String line;
      Message msg;
      while ((line = is.readLine()) != null) {

        if ((msg = readMessage(line)) == null)
          continue;

        // Check black list
        if (blm.contains(msg.getAbonent()))
          continue;

        // Check region
        Region r = (Region) rtree.getValue(msg.getAbonent());
        if (r == null) {
          category.warn("WARN: rtree doesn't contains: " + msg.getAbonent());
          continue;
        }

        Integer regionId = new Integer(r.getId());
        if (!d.splitByRegions) {
          r = new Region("All regions");
          r.setTimezone(TimeZone.getDefault());
          regionId = new Integer(Integer.MIN_VALUE);
        }

        // Get task part for region
        TaskPart taskPart = (TaskPart) tasks.get(regionId);
        if (taskPart == null) {
          if (category.isDebugEnabled())
            category.debug("New region '" + r.getName() + "' found for file: " + fileName);
          taskPart = new TaskPart(task, r, smeContext);
          tasks.put(regionId, taskPart);
        }

        // Add message to task part
        taskPart.addMessage(msg);
      }

      for (Iterator iter = tasks.values().iterator(); iter.hasNext();)
        ((TaskPart) iter.next()).done();

      if (category.isDebugEnabled())
        category.debug("Task's generation finished: " + fileName);

      if (!file.renameTo(new File(fileName + ".processed")))
        category.error("Can't rename initial file: " + fileName);

    } catch (Exception e) {
      category.error(e, e);

      for (Iterator iter = tasks.values().iterator(); iter.hasNext();) {
        TaskPart part = (TaskPart) iter.next();
        try {
          part.cancel();
        } catch (Exception ex) {
          category.error("Unable to cancel task part: " + part.task.getName(), e);
        }
      }

      if (!file.renameTo(new File(fileName + ".failed")))
        category.warn("Can't rename file to '.failed'");

      saveTaskFail(e, fileName);
    } finally {
      try {
        if (is != null)
          is.close();
      } catch (IOException e) {
      }
    }
  }

  private void initiateRadixTree(TemplatesRadixTree tree, User user) {
    Collection regions = appContext.getRegionsManager().getRegions();
    boolean admin = user.getRoles().contains(InfoSmeBean.INFOSME_ADMIN_ROLE);

    Region r;
    for (Iterator regionsIter = regions.iterator(); regionsIter.hasNext();) {
      r = (Region) regionsIter.next();
      if (admin || user.getPrefs().isInfoSmeRegionAllowed(String.valueOf(r.getId()))) {

        for (Iterator subjectsIter = r.getSubjects().iterator(); subjectsIter.hasNext();) {
          String subjectName = (String) subjectsIter.next();
          Subject s = appContext.getRouteSubjectManager().getSubjects().get(subjectName);
          tree.add(s.getMasks().getNames(), r);
        }
      }
    }
  }

  private void saveTaskFail(Exception e, String file) {
    PrintWriter out = null;

    try {
      out = new PrintWriter(new FileOutputStream(file + ".stacktrace"));
      e.printStackTrace(out);
    } catch (FileNotFoundException ex) {
      category.error(ex, ex);
    } finally {
      if (out != null)
        out.close();
    }
  }

  private static class Distribution {
    Task task;
    User user;
    String filename;
    boolean splitByRegions;

    public Distribution(Task task, User user, String filename) {
      this.task = task;
      this.user = user;
      this.filename = filename;
    }

  }

  private static class TaskPart {

    private Task task;
    private final List messageBuffer;
    private final int maxMessagesPerSecond;
    private Date startDate;
    private final InfoSmeContext smeContext;

    public TaskPart(Task originalTask, Region r, InfoSmeContext smeContext) throws AdminException {
      this.maxMessagesPerSecond = smeContext.getLicense().getMaxSmsThroughput();
      this.messageBuffer = new ArrayList(maxMessagesPerSecond + 1);
      this.smeContext = smeContext;
      this.startDate = new Date();

      task = originalTask.cloneTask();
      task.setName(task.getName() + '(' + r.getName() + ')');
      task.setMessagesHaveLoaded(false);

      // Convert date to TZ
      task.setActivePeriodStart(r.getLocalTime(task.getActivePeriodStart()));
      task.setActivePeriodEnd(r.getLocalTime(task.getActivePeriodEnd()));

      // Create new task in InfoSme or use existed task
      if (smeContext.getInfoSmeConfig().containsTaskWithName(task.getName())) {
        Task oldTask = smeContext.getInfoSmeConfig().getTaskByName(task.getName());
        task = oldTask;
      } else {
        smeContext.getInfoSmeConfig().addAndApplyTask(task);
        smeContext.getInfoSme().addTask(task.getId());
      }
    }

    public void addMessage(Message m) throws AdminException {
      m.setSendDate(startDate);
      messageBuffer.add(m);

      if (messageBuffer.size() == maxMessagesPerSecond)
        flushBuffer();
    }

    private void flushBuffer() throws AdminException {
      if (!messageBuffer.isEmpty()) {
        smeContext.getInfoSme().addDeliveryMessages(task.getId(), messageBuffer);
        messageBuffer.clear();
      }
      startDate.setTime(startDate.getTime() + 1000);
    }

    public void done() throws AdminException {
      flushBuffer();

      smeContext.getInfoSme().endDeliveryMessageGeneration(task.getId());
      task.setMessagesHaveLoaded(true);
      smeContext.getInfoSmeConfig().addAndApplyTask(task);
    }

    public void cancel() throws AdminException {
      if (task.getId() != null) {
        smeContext.getInfoSmeConfig().removeAndApplyTask(task.getOwner(), task.getId());
        smeContext.getInfoSme().removeTask(task.getId());
      }
    }
  }

}