package ru.novosoft.smsc.infosme.backend.commands;

import org.apache.log4j.Category;
import ru.novosoft.smsc.admin.console.CommandContext;
import ru.novosoft.smsc.admin.console.commands.infosme.Distribution;
import ru.novosoft.smsc.admin.console.commands.infosme.InfoSmeCommands;
import ru.novosoft.smsc.admin.users.User;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.infosme.backend.InfoSmeContext;
import ru.novosoft.smsc.infosme.backend.Message;
import ru.novosoft.smsc.infosme.backend.config.tasks.Task;
import ru.novosoft.smsc.infosme.beans.InfoSmeBean;
import ru.novosoft.smsc.jsp.SMSCAppContext;

import java.io.File;
import java.io.IOException;
import java.util.*;

/**
 * User: artem
 * Date: Aug 6, 2007
 */

public class InfoSmeCommandsImpl implements InfoSmeCommands {

  private static Category log = Category.getInstance(InfoSmeCommandsImpl.class);

  private static boolean checkUserPermissions(CommandContext ctx) {
    User user = getUser(ctx);
    if (!isUserAdmin(user) && !isUserMarket(user)) {
      ctx.setMessage("User is not allowed to perform this action.");
      ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
      return false;
    }
    return true;
  }

  private static boolean checkInfoSmeOnline(CommandContext ctx, InfoSmeContext context) {
    if (!context.getInfoSme().getInfo().isOnline()) {
      ctx.setMessage("InfoSme is not started");
      ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
      return false;
    }
    return true;
  }

  private static boolean checkTaskOwner(CommandContext ctx, InfoSmeContext context, String taskId) {
    // Check task exists
    if(!context.getInfoSmeConfig().containsTaskWithId(taskId)) {
      ctx.setMessage("Task doesn't exist in InfoSme with id: " + taskId);
      ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
      return false;
    }

    // Check owner
    User user = getUser(ctx);
    if (!isUserAdmin(user)) {
      Task t = context.getInfoSmeConfig().getTask(taskId);
      if (!t.getOwner().equals(user.getName())) {
        ctx.setMessage("User is not allowed to perform this action.");
        ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
        return false;
      }
    }

    return true;
  }

  public void importFile(CommandContext ctx, String file) {
    try {
      // Check user
      if (!checkUserPermissions(ctx))
        return;

      File f = new File(file);
      if (!f.exists()) {
        ctx.setMessage("File " + file + " does not exist");
        ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
        return;
      }

      final SMSCAppContext appContext = ctx.getOwner().getContext();
      final InfoSmeContext context = InfoSmeContext.getInstance(appContext, "InfoSme");

      if (!checkInfoSmeOnline(ctx, context))
        return;

      // Prepare task
      User user = getUser(ctx);
      List tasks = context.getInfoSmeConfig().getTasks(user.getLogin());
      String name = new File(file).getName();
      int ind = name.lastIndexOf('.');
      if (ind > 0)
        name = name.substring(0, ind);

      Task task = null;
      for (Iterator iter = tasks.iterator(); iter.hasNext();) {
        Task t = (Task)iter.next();
        if (t.getName().equals(name)) {
          task = t;
          break;
        }
      }
      if (task == null)
        task = context.getInfoSmeConfig().createTask(user, name);

      if(!context.getTaskManager().addTask(task, user, file)) {
        throw new AdminException("Can't add task: "+file);
      }
      ctx.setMessage("File " + file + " was added to process queue");
      ctx.setStatus(CommandContext.CMD_OK);
    } catch (Exception e) {
      e.printStackTrace();
      log.error(e);
      ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
    }
  }



  public void exportStat(CommandContext ctx, String taskName, String file, Date startDate) {
    try{
      // Check user
      if (!checkUserPermissions(ctx))
        return;

      final SMSCAppContext appContext = ctx.getOwner().getContext();
      final InfoSmeContext context = InfoSmeContext.getInstance(appContext, "InfoSme");

      if (!checkInfoSmeOnline(ctx, context))
        return;

      switch(context.getExportStatManager().addExportTask(taskName, file, startDate)) {
        case -1 :
          ctx.setMessage("Can't export stats for task: "+taskName);
          ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
          break;
        case 0 :
          ctx.setMessage("Task is not processed: "+taskName);
          ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
          break;
        case 1 :
          ctx.setMessage("Export is started. See results later in :"+file);
          ctx.setStatus(CommandContext.CMD_OK);
          break;
      }

    }catch(IOException e){
      log.error(e,e);
      ctx.setMessage("Problems with creation file: "+file);
      ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
    }catch(Exception e){
      log.error(e,e);
      ctx.setMessage("Can't export stats for task: "+taskName);
      ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
    }
  }

  public void resendMessage(CommandContext ctx, String msisdn, String taskId, String text) {
    try {
      // Check user
      if (!checkUserPermissions(ctx))
        return;

      final SMSCAppContext appContext = ctx.getOwner().getContext();
      final InfoSmeContext context = InfoSmeContext.getInstance(appContext, "InfoSme");

      if (!checkInfoSmeOnline(ctx, context))
        return;

//      String storeDir = context.getConfig().getString("InfoSme.storeLocation");
//
//      MessageDataSource ds = new MessageDataSource(context.getConfig(), storeDir);
//      long id = ds.getMessageId(msisdn, taskId);
//      if(id == -1) {
//        throw new Exception("Message not found for taskId="+taskId+" msisdn="+msisdn);
//      }
      final Message message = new Message();
      message.setAbonent(msisdn);
      message.setMessage(text.replaceAll("\\\\r","").replaceAll("\\\\n",System.getProperty("line.separator")));
      message.setSendDate(new Date());
      message.setState(Message.State.NEW);
      message.setTaskId(taskId);

      Collection mesList = new LinkedList();
      mesList.add(message);

      context.getInfoSme().addDeliveryMessages(taskId, mesList);
//      context.getInfoSme().changeDeliveryTextMessage(taskId, Long.toString(id), Message.State.NEW, new Date(), null, text); //resendMessages(taskId, Long.toString(id), Message.State.NEW, new Date());

      ctx.setMessage("OK");
      ctx.setStatus(CommandContext.CMD_OK);
    } catch (Exception e) {
      e.printStackTrace();
      log.error(e);
      ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
    }

  }

  public void removeTask(CommandContext ctx, String taskId) {
    try {
      // Check user
      if (!checkUserPermissions(ctx))
        return;

      final SMSCAppContext appContext = ctx.getOwner().getContext();
      final InfoSmeContext context = InfoSmeContext.getInstance(appContext, "InfoSme");

      if (!checkInfoSmeOnline(ctx, context) || !checkTaskOwner(ctx, context, taskId))
        return;

      // Remove task
      context.getInfoSmeConfig().removeAndApplyTask(getUser(ctx).getLogin(), taskId);
      context.getInfoSme().removeTask(taskId);

      ctx.setMessage("OK");
      ctx.setStatus(CommandContext.CMD_OK);
    } catch (Exception e) {
      log.error("Unable to remove task:" + taskId, e);
      log.error(e);
      ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
    }
  }

  public void createTask(CommandContext ctx, Distribution d) {
    try {
      // Check user
      if (!checkUserPermissions(ctx))
        return;

      try{
        validateDistribution(d);
      } catch(Exception e) {
        ctx.setMessage("Wrong reguest command, some parameters type is unsupported: "+e.getMessage());
        ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
        return;
      }

      File f = new File(d.getFile());
      if (!f.exists()) {
        ctx.setMessage("File " + d.getFile() + " does not exist");
        ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
        return;
      }

      final SMSCAppContext appContext = ctx.getOwner().getContext();
      final InfoSmeContext context = InfoSmeContext.getInstance(appContext, "InfoSme");

      if (!checkInfoSmeOnline(ctx, context))
        return;

      // Create task
      User user = getUser(ctx);
      Task t = createTask(context, d, user);

      if(!context.getTaskManager().addTask(t, user, d.getFile())) {
        throw new AdminException("Can't add task: "+d.getFile());
      }

      ctx.setMessage(t.getId());
      ctx.setStatus(CommandContext.CMD_OK);

    } catch (Exception e) {
      e.printStackTrace();
      log.error(e,e);
      ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
    }
  }

  public void alterTask(CommandContext ctx, Distribution d, String taskId) {
    // Check user
    if (!checkUserPermissions(ctx))
      return;

    try{
      validateDistribution(d);
      validateNull(taskId, "TaskId");
    } catch(Exception e) {
      ctx.setMessage("Wrong reguest command, some parameters type is unsupported: "+e.getMessage());
      ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
      return;
    }

    try{
      final SMSCAppContext appContext = ctx.getOwner().getContext();
      final InfoSmeContext context = InfoSmeContext.getInstance(appContext, "InfoSme");

      if (!checkInfoSmeOnline(ctx, context) || !checkTaskOwner(ctx, context, taskId))
        return;

      // Alter task
      Task t = context.getInfoSmeConfig().getTask(taskId);
      alterTask(t, d);

      // Save task
      context.getInfoSmeConfig().addAndApplyTask(t);
      context.getInfoSme().changeTask(taskId);

    } catch (Exception e) {
      e.printStackTrace();
      log.error(e,e);
      ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
    }
  }

  public void getStatus(CommandContext ctx, String taskId) {
    try{
      // Check user
      if (!checkUserPermissions(ctx))
        return;

      final SMSCAppContext appContext = ctx.getOwner().getContext();
      final InfoSmeContext context = InfoSmeContext.getInstance(appContext, "InfoSme");

      if (!checkInfoSmeOnline(ctx, context) && !checkTaskOwner(ctx, context, taskId))
        return;

      Task t = context.getInfoSmeConfig().getTask(taskId);
      String status = Boolean.toString(t.isMessagesHaveLoaded());

      ctx.setMessage(status);
      ctx.setStatus(CommandContext.CMD_OK);

    } catch (Exception e) {
      e.printStackTrace();
      ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
    }
  }

  private static User getUser(CommandContext ctx) {
    SMSCAppContext appContext = ctx.getOwner().getContext();
    return appContext.getUserManager().getUser(ctx.getSession().getUserName());
  }

  private static boolean isUserAdmin(User user) {
    return user.getRoles().contains(InfoSmeBean.INFOSME_ADMIN_ROLE);
  }

  private static boolean isUserMarket(User user) {
    return user.getRoles().contains(InfoSmeBean.INFOSME_MARKET_ROLE);
  }

  private static void validateDistribution(Distribution d) {
    validateNull(d,"Distribution");
    validateNull(d.getDateBegin(),"dateBegin");
    validateNull(d.getDateEnd(),"dateEnd");
    validateNull(d.getTimeBegin(),"timeBegin");
    validateNull(d.getTimeEnd(),"timeEnd");
    validateNull(d.getDays(),"days");
    validateNull(d.isTxmode(),"txmode");
    validateNull(d.getAddress(),"address");
    validateNull(d.getTaskName(),"taskName");
  }

  private static void validateNull(Object obj, String name){
    if(obj==null) {
      throw new IllegalArgumentException("Param empty: "+name);
    }
  }

  private static Task createTask(InfoSmeContext ctx, Distribution distr, User owner) throws AdminException {
    Task task = ctx.getInfoSmeConfig().createTask(owner, distr.getTaskName());
    return alterTask(task, distr);
  }

  private static Task alterTask(Task task, Distribution distr) {
    task.setName(distr.getTaskName());
    Set days = distr.getDays();
    List activeDays = new LinkedList();

    if(days != null && !days.isEmpty()) {
      Iterator iter = days.iterator();
      while(iter.hasNext()) {
        activeDays.add(Task.WEEK_DAYS[((Integer)iter.next()).intValue()]);
      }
    }
    task.setName(distr.getTaskName());
    task.setActiveWeekDays(activeDays);
    task.setActiveWeekDaysSet(activeDays);

    task.setStartDate(distr.getDateBegin());
    task.setEndDate(distr.getDateEnd());
    task.setActivePeriodStart(distr.getTimeBegin().getTime());
    task.setActivePeriodEnd(distr.getTimeEnd().getTime());
    task.setAddress(distr.getAddress());
    task.setTransactionMode(distr.isTxmode().booleanValue());
    return task;
  }

}