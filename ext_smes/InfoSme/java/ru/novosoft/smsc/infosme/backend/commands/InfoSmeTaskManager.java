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

public class InfoSmeTaskManager implements Runnable{

  private static final Category category = Category.getInstance(InfoSmeTaskManager.class);

  private final LinkedList distributions = new LinkedList();

  private final static String SME_ID = "InfoSme";

  private final SMSCAppContext appContext;

  public InfoSmeTaskManager(SMSCAppContext appContext) {
    this.appContext = appContext;
  }

  private void removeTask(Task task, InfoSmeContext infoSmeContext) {
    if(task.getId() == null) {
      return;
    }
    try {
      infoSmeContext.getInfoSmeConfig().removeAndApplyTask(task.getOwner(), task.getId());
      infoSmeContext.getInfoSme().removeTask(task.getId());
    } catch (Throwable e) {
      category.error(e,e);
    }
  }


  private boolean started = false;

  public boolean addTask(Task task, User user, String filename) {
    if(!started) {
      return false;
    }
    if(task == null || user == null || filename == null) {
      throw new IllegalArgumentException("Some arguments are null");
    }
    Distribution d = new Distribution(task, user, filename);
    synchronized(lock) {
      distributions.addLast(d);
    }
    return true;
  }


  private final Object lock = new Object();

  private final Object shutdownLock = new Object();

  /** @noinspection EmptyCatchBlock*/
  public void run() {
    started = true;
    if (category.isDebugEnabled()) {
      category.debug("InfoSmeTaskManager is started");
    }
    while(started) {
      try{
        Distribution d = null;
        synchronized(lock) {
          if(!distributions.isEmpty()) {
            d = (Distribution)distributions.removeFirst();
          }
        }
        if(d != null) {
          synchronized(shutdownLock) {
            handle(d);
          }
        } else {
          try {
            Thread.sleep(5000);
          } catch (InterruptedException e) {}
        }
      }catch(Throwable e) {
        category.error(e,e);
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
    synchronized(shutdownLock){
      if(category.isDebugEnabled()) {
        category.debug("InfoSmeTaskManager is shutdowned");
      }
    }
  }

  /** @noinspection EmptyCatchBlock*/
  private void handle(Distribution d) throws Config.WrongParamTypeException, IOException, ParserConfigurationException,
      AdminException, Config.ParamNotFoundException, SAXException {

    InfoSmeContext smeContext = InfoSmeContext.getInstance(appContext, SME_ID);

    if(smeContext == null) {
      throw new AdminException("Infosme is offline");
    }

    String fileName = d.filename;
    User user = d.user;
    Task task = d.task;

    File file = new File(fileName);
    if(!file.exists()) {
      category.warn("File to process doesn't exist: "+fileName);
      return;
    }

    TemplatesRadixTree rtree = new TemplatesRadixTree();
    initiateRadixTree(rtree, user);

    BufferedReader is = null;
    try {
      task.setMessagesHaveLoaded(false);

      if (task.getId() == null || !smeContext.getInfoSmeConfig().containsTaskWithId(task.getId())) {
        smeContext.getInfoSmeConfig().addAndApplyTask(task);
        smeContext.getInfoSme().addTask(task.getId());
      }

      if(category.isDebugEnabled()) {
        category.debug("Start task's generation for: " + fileName);
      }

      is = new BufferedReader(new InputStreamReader(new FileInputStream(file), Functions.getLocaleEncoding()));

      final int maxMessagesPerSecond = smeContext.getMaxSmsThroughput();

      long currentTime = task.getStartDate() == null ? System.currentTimeMillis() : task.getStartDate().getTime();
      List messages = new ArrayList(maxMessagesPerSecond + 1);
      boolean res = getMessages(is, maxMessagesPerSecond, new Date(currentTime), rtree, smeContext, messages);
      while (res || !messages.isEmpty()) {
        if (!messages.isEmpty()) {
          smeContext.getInfoSme().addDeliveryMessages(task.getId(), messages);
          messages.clear();
          currentTime += 1000;
        }
        res = getMessages(is, maxMessagesPerSecond,  new Date(currentTime), rtree, smeContext, messages);
      }

      smeContext.getInfoSme().endDeliveryMessageGeneration(task.getId());

      task.setMessagesHaveLoaded(true);
      smeContext.getInfoSmeConfig().addAndApplyTask(task);

      if(category.isDebugEnabled()) {
        category.debug("Task's generation finished: " + fileName);
      }
      if (!file.renameTo(new File(fileName + ".processed"))) {
        category.error("Can't rename initial file: "+fileName);
      }
    } catch (Exception e) {
      category.error(e,e);
      removeTask(task, smeContext);
      if (!file.renameTo(new File(fileName + ".failed"))) {
        category.warn("Can't rename file to '.failed'");
      }
      saveTaskFail(e, fileName);
    } finally {
      try {
        if (is != null)
          is.close();
      } catch (IOException e) {}
    }
  }

  private void initiateRadixTree(TemplatesRadixTree tree, User user) {
    Collection regions = appContext.getRegionsManager().getRegions();
    boolean admin = user.getRoles().contains(InfoSmeBean.INFOSME_ADMIN_ROLE);

    Region r;
    for (Iterator regionsIter = regions.iterator(); regionsIter.hasNext();) {
      r = (Region)regionsIter.next();
      if (admin || user.getPrefs().isInfoSmeRegionAllowed(String.valueOf(r.getId()))) {

        for (Iterator subjectsIter = r.getSubjects().iterator(); subjectsIter.hasNext();) {
          String subjectName = (String)subjectsIter.next();
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

  private boolean getMessages(BufferedReader is, int limit, Date sendDate,
                              TemplatesRadixTree rtree, InfoSmeContext smeContext, List list) throws IOException, AdminException {
    String line = null;
    BlackListManager blm = smeContext.getBlackListManager();
    for (int i=0; i < limit && ((line = is.readLine()) != null); i++) {

      int index = line.indexOf('|');
      if (index > 0) {
        String msisdn = line.substring(0, index);
        if (msisdn.charAt(0) != '+')
          msisdn = '+' + msisdn;
        if (blm.contains(msisdn)) {
          continue;
        }
        if (rtree.getValue(msisdn) == null) {
          category.warn("WARN: rtree doesn't contains: "+msisdn);
          continue;
        }

        String text = line.substring(index+1);
        text = text.replaceAll("\\\\r","").replaceAll("\\\\n",System.getProperty("line.separator"));

        final Message msg = new Message();
        msg.setAbonent(msisdn);
        msg.setMessage(text);
        msg.setState(Message.State.NEW);
        msg.setSendDate(sendDate);
        list.add(msg);
      }
    }

    return line != null;

  }

  private static class Distribution {
    Task task;
    User user;
    String filename;

    public Distribution(Task task, User user, String filename) {
      this.task = task;
      this.user = user;
      this.filename = filename;
    }

  }

}