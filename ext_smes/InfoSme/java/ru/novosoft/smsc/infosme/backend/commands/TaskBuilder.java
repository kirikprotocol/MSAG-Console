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

import java.io.*;
import java.util.*;

/**
 * User: artem
 * Date: Aug 6, 2007
 */

public class TaskBuilder extends Thread {

  private final Task task;
  private final String file;
  private final InfoSmeContext smeContext;
  private final SMSCAppContext appContext;
  private final User user;

  public TaskBuilder(SMSCAppContext appContext, InfoSmeContext smeContext, Task task, User user, String file) {
    this.smeContext = smeContext;
    this.appContext = appContext;
    this.user = user;
    this.task = task;
    this.file = file;
  }

  private void removeTask(Task task) {
    try {
      smeContext.getInfoSmeConfig().removeAndApplyTask(task.getOwner(), task.getId());
      smeContext.getInfoSme().removeTask(task.getId());
    } catch (Throwable e) {
      e.printStackTrace();
    }
  }

  public void run() {

    System.out.println("Task builder started");

    // Validate task
    if (smeContext.getInfoSmeConfig().containsTaskWithName(task.getName())) {
      System.out.println("Task with name '" + task.getName() + "' already exists");
      return;
    }

    System.out.println("Copy file: " + file);

    String processedFile = file + ".processed";

    if (!new File(file).renameTo(new File(processedFile))) {
      System.out.println("Can't rename initial file");
      return;
    }

    TemplatesRadixTree rtree = new TemplatesRadixTree();
    initiateRadixTree(rtree);

    BufferedReader is = null;
    try {
      task.setMessagesHaveLoaded(false);

      smeContext.getInfoSmeConfig().addAndApplyTask(task);

      System.out.println("Task generation....");

      is = new BufferedReader(new InputStreamReader(new FileInputStream(processedFile), Functions.getLocaleEncoding()));
      smeContext.getInfoSme().addTask(task.getId());

      final int maxMessagesPerSecond = smeContext.getInfoSmeConfig().getMaxMessagesPerSecond();

      long currentTime = task.getStartDate() == null ? System.currentTimeMillis() : task.getStartDate().getTime();
      Collection messages = getMessages(is, maxMessagesPerSecond, new Date(currentTime), rtree);
      while (messages != null && !messages.isEmpty()) {
        smeContext.getInfoSme().addDeliveryMessages(task.getId(), messages);
        currentTime += 1000;
        messages = getMessages(is, maxMessagesPerSecond,  new Date(currentTime), rtree);
      }

      smeContext.getInfoSme().endDeliveryMessageGeneration(task.getId());

      System.out.println("Task generation ok");

      task.setMessagesHaveLoaded(true);
      smeContext.getInfoSmeConfig().addAndApplyTask(task);

    } catch (Exception e) {
      e.printStackTrace();
      removeTask(task);
      if (!new File(processedFile).renameTo(new File(file + ".failed")))
        System.out.println("Can't rename file to '.failed'");
      saveTaskFail(e);
    } finally {
      try {
        if (is != null)
          is.close();
      } catch (IOException e) {
        e.printStackTrace();
      }
    }
  }

  private void initiateRadixTree(TemplatesRadixTree tree) {
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

  private void saveTaskFail(Exception e) {
    PrintWriter out = null;

    try {
      out = new PrintWriter(new FileOutputStream(file + ".stacktrace"));
      e.printStackTrace(out);
    } catch (FileNotFoundException ex) {
      ex.printStackTrace();
    } finally {
      if (out != null)
        out.close();
    }
  }

  public List getMessages(BufferedReader is, int limit, Date sendDate, TemplatesRadixTree rtree) throws IOException, AdminException {
    final List list = new ArrayList(limit);

    String line;
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
            System.out.println("WARN: rtree doesn't contains: "+msisdn);
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

    return list;
  }

}