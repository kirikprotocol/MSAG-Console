package ru.novosoft.smsc.infosme.backend.commands;

import ru.novosoft.smsc.admin.console.commands.infosme.InfoSmeDistr;
import ru.novosoft.smsc.admin.console.CommandContext;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.infosme.backend.InfoSmeContext;
import ru.novosoft.smsc.infosme.backend.tables.tasks.TaskDataSource;
import ru.novosoft.smsc.util.StringEncoderDecoder;

import java.io.File;

/**
 *
 * User: alkhal
 * Date: 22.10.2008
 *
 */

public class InfoSmeDistrImpl implements InfoSmeDistr {

  public void createDistribution(CommandContext ctx, String fileName, String dateBegin, String dateEnd,
                                 String timeBegin, String timeEnd, String days, Boolean txmode, String address) {
    try {
      TaskProps taskProps;
      try{
        validateNull(dateBegin,"dateBegin");
        validateNull(dateEnd,"dateEnd");
        validateNull(timeBegin,"timeBegin");
        validateNull(timeEnd,"timeEnd");
        validateNull(days,"days");
        validateNull(txmode,"txmode");
        validateNull(address,"address");
      } catch(Exception e) {
        ctx.setMessage("Wrong reguest command, some parameters type is unsupported: "+e.getMessage());
        ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
        return;
      }
      File f = new File(fileName);
      if (!f.exists()) {
        ctx.setMessage("File " + fileName + " does not exist");
        ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
        return;
      }


      final SMSCAppContext appContext = ctx.getOwner().getContext();
      if (!appContext.getSmeManager().smeStatus("InfoSme").isConnected()) {
        ctx.setMessage("InfoSme is not started");
        ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
        return;
      }

      final InfoSmeContext context = InfoSmeContext.getInstance(appContext, "InfoSme");

      taskProps = new TaskProps();
      taskProps.setAddress(address);
      taskProps.setDateBegin(dateBegin);
      taskProps.setDateEnd(dateEnd);
      taskProps.setDays(days);
      taskProps.setTimeBegin(timeBegin);
      taskProps.setTimeEnd(timeEnd);
      taskProps.setTxmode(txmode.booleanValue());

      TaskBuilder taskBuilder = new TaskBuilder(fileName, context, taskProps);
      String taskId;
      if((taskId = taskBuilder.getTaskId())==null) {
        throw new Exception("InfoSme Internal error");
      }
      taskBuilder.start();
      ctx.setMessage(taskId);
      ctx.setStatus(CommandContext.CMD_OK);
    } catch (Exception e) {
      e.printStackTrace();
      ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
    }
  }

  public void getStatus(CommandContext ctx, String taskId) {
    try{
      final SMSCAppContext appContext = ctx.getOwner().getContext();
      if (!appContext.getSmeManager().smeStatus("InfoSme").isConnected()) {
        ctx.setMessage("InfoSme is not started");
        ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
        return;
      }

      final InfoSmeContext smeContext = InfoSmeContext.getInstance(appContext, "InfoSme");
      final String prefix = TaskDataSource.TASKS_PREFIX + '.' + StringEncoderDecoder.encodeDot(taskId);
      String status = smeContext.getConfig().getString(prefix + ".status");
      if(status==null) {
        status="";
      }
      ctx.setMessage(status);
      ctx.setStatus(CommandContext.CMD_OK);

    } catch (Exception e) {
      e.printStackTrace();
      ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
    }
  }

  private void validateNull(Object obj, String name) throws Exception{
    if((obj==null)||(obj.toString().equals(""))) {
      throw new Exception("Param empty: "+name);
    }
  }
}
