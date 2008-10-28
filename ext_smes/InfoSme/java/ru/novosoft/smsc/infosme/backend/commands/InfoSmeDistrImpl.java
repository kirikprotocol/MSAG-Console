package ru.novosoft.smsc.infosme.backend.commands;

import ru.novosoft.smsc.admin.console.commands.infosme.InfoSmeDistr;
import ru.novosoft.smsc.admin.console.commands.infosme.Distribution;
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

  public void createDistribution(CommandContext ctx, Distribution d) {
    try {
      try{
        validateNull(d.getDateBegin(),"dateBegin");
        validateNull(d.getDateEnd(),"dateEnd");
        validateNull(d.getTimeBegin(),"timeBegin");
        validateNull(d.getTimeEnd(),"timeEnd");
        validateNull(d.getDays(),"days");
        validateNull(d.isTxmode(),"txmode");
        validateNull(d.getAddress(),"address");
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
      if (!appContext.getSmeManager().smeStatus("InfoSme").isConnected()) {
        ctx.setMessage("InfoSme is not started");
        ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
        return;
      }

      final InfoSmeContext context = InfoSmeContext.getInstance(appContext, "InfoSme");


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
