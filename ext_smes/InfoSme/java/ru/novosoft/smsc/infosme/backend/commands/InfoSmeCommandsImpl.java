package ru.novosoft.smsc.infosme.backend.commands;

import org.apache.log4j.Category;
import ru.novosoft.smsc.admin.console.CommandContext;
import ru.novosoft.smsc.admin.console.commands.infosme.Distribution;
import ru.novosoft.smsc.admin.console.commands.infosme.InfoSmeCommands;
import ru.novosoft.smsc.infosme.backend.InfoSmeContext;
import ru.novosoft.smsc.infosme.backend.Message;
import ru.novosoft.smsc.infosme.backend.Task;
import ru.novosoft.smsc.infosme.backend.tables.messages.MessageDataSource;
import ru.novosoft.smsc.infosme.backend.tables.tasks.TaskDataSource;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.util.StringEncoderDecoder;
import ru.novosoft.smsc.util.config.Config;

import java.io.File;
import java.util.Date;
import java.util.LinkedList;
import java.util.Collection;

/**
 * User: artem
 * Date: Aug 6, 2007
 */

public class InfoSmeCommandsImpl implements InfoSmeCommands {


  private static Category log = Category.getInstance(InfoSmeCommandsImpl.class);

  public void importFile(CommandContext ctx, String file) {
    try {

      File f = new File(file);
      if (!f.exists()) {
        ctx.setMessage("File " + file + " does not exist");
        ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
        return;
      }

      final SMSCAppContext appContext = ctx.getOwner().getContext();
      final InfoSmeContext context = InfoSmeContext.getInstance(appContext, "InfoSme");

      if (!context.getInfoSme().getInfo().isOnline()) {
        ctx.setMessage("InfoSme is not started");
        ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
        return;
      }

      new TaskBuilder(file, context).start();
      ctx.setMessage("File " + file + " was added to process queue");
      ctx.setStatus(CommandContext.CMD_OK);
    } catch (Exception e) {
      e.printStackTrace();
      log.error(e);
      ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
    }
  }

  public void resendMessage(CommandContext ctx, String msisdn, String taskId, String text) {
    try {
      final SMSCAppContext appContext = ctx.getOwner().getContext();
      final InfoSmeContext context = InfoSmeContext.getInstance(appContext, "InfoSme");

      if (!context.getInfoSme().getInfo().isOnline()) {
        ctx.setMessage("InfoSme is not started");
        ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
        return;
      }

//      String storeDir = context.getConfig().getString("InfoSme.storeLocation");
//
//      MessageDataSource ds = new MessageDataSource(context.getConfig(), storeDir);
//      long id = ds.getMessageId(msisdn, taskId);
//      if(id == -1) {
//        throw new Exception("Message not found for taskId="+taskId+" msisdn="+msisdn);
//      }
      final Message message = new Message();
      message.setAbonent(msisdn);
      message.setMessage(text);
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
      final SMSCAppContext appContext = ctx.getOwner().getContext();
      final InfoSmeContext context = InfoSmeContext.getInstance(appContext, "InfoSme");
      if (!context.getInfoSme().getInfo().isOnline()) {
        ctx.setMessage("InfoSme is not started");
        ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
        return;
      }

      Task t = new Task(taskId);
      t.removeFromConfig(context.getConfig());
      context.getInfoSme().removeTask(t.getId());
      context.getConfig().save();

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
      if (!context.getInfoSme().getInfo().isOnline()) {
        ctx.setMessage("InfoSme is not started");
        ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
        return;
      }

      TaskBuilder taskBuilder = new TaskBuilder(context, d);
      String taskId;
      if((taskId = taskBuilder.getTaskId())==null) {
        throw new Exception("InfoSme Internal error");
      }
      taskBuilder.start();
      ctx.setMessage(taskId);
      ctx.setStatus(CommandContext.CMD_OK);
    } catch (Exception e) {
      e.printStackTrace();
      log.error(e,e);
      ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
    }
  }

  public void alterTask(CommandContext ctx, Distribution d, String taskId) {
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
      if (!context.getInfoSme().getInfo().isOnline()) {
        ctx.setMessage("InfoSme is not started");
        ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
      } else{
        final Config config = context.getConfig();
        Task task = new Task(taskId);
        if(!task.isContainsInConfig(config)) {
          ctx.setMessage("Task doesn't exist in InfoSme with id: " + taskId);
          ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
          return;
        }
        task.resetTask(false);
        task.importFromDistribution(d);
        task.removeFromConfig(config);
        task.storeToConfig(config);
        config.save();
        context.getInfoSme().changeTask(taskId);
      }
    } catch (Exception e) {
      e.printStackTrace();
      log.error(e,e);
      ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
    }
  }

  public void getStatus(CommandContext ctx, String taskId) {
    try{
      final SMSCAppContext appContext = ctx.getOwner().getContext();
      final InfoSmeContext context = InfoSmeContext.getInstance(appContext, "InfoSme");
      if (!context.getInfoSme().getInfo().isOnline()) {
        ctx.setMessage("InfoSme is not started");
        ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
        return;
      }

      final InfoSmeContext smeContext = InfoSmeContext.getInstance(appContext, "InfoSme");
      final String prefix = TaskDataSource.TASKS_PREFIX + '.' + StringEncoderDecoder.encodeDot(taskId);
      String status;
      try{
        boolean loaded = smeContext.getConfig().getBool(prefix + ".messagesHaveLoaded");
        status = Boolean.toString(loaded);
      } catch(Exception e) {
        status="error";
      }
      ctx.setMessage(status);
      ctx.setStatus(CommandContext.CMD_OK);

    } catch (Exception e) {
      e.printStackTrace();
      ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
    }
  }

  private void validateDistribution(Distribution d) {
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

  private void validateNull(Object obj, String name){
    if(obj==null) {
      throw new IllegalArgumentException("Param empty: "+name);
    }
  }

}
