package ru.novosoft.smsc.infosme.backend.commands;

import ru.novosoft.smsc.admin.console.commands.infosme.InfoSmeCreateDistr;
import ru.novosoft.smsc.admin.console.CommandContext;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.infosme.backend.InfoSmeContext;

import java.util.*;
import java.io.File;

/**
 *
 * User: alkhal
 * Date: 22.10.2008
 *
 */

public class InfoSmeCreateDistrImpl implements InfoSmeCreateDistr {

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
      new TaskBuilder(fileName, context, taskProps).start();
      ctx.setMessage("File " + fileName + " was added to process queue");
      ctx.setStatus(CommandContext.CMD_OK);
    } catch (Exception e) {
      e.printStackTrace();
    }
  }


  private void validateNull(Object obj, String name) throws Exception{
    if((obj==null)||(obj.toString().equals(""))) {
      throw new Exception("Param empty: "+name);
    }
  }
}
