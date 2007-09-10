package ru.novosoft.smsc.infosme.backend.commands;

import ru.novosoft.smsc.admin.console.CommandContext;
import ru.novosoft.smsc.admin.console.commands.infosme.InfoSmeCommands;
import ru.novosoft.smsc.admin.service.ServiceInfo;
import ru.novosoft.smsc.admin.route.SmeStatus;
import ru.novosoft.smsc.infosme.backend.InfoSmeContext;
import ru.novosoft.smsc.jsp.SMSCAppContext;

import java.io.File;

/**
 * User: artem
 * Date: Aug 6, 2007
 */

public class InfoSmeCommandsImpl implements InfoSmeCommands {

  public void importFile(CommandContext ctx, String file) {
    try {

      File f = new File(file);
      if (!f.exists()) {
        ctx.setMessage("File " + file + " does not exist");
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

      new TaskBuilder(file, context).start();
      ctx.setMessage("File " + file + " was added to process queue");
      ctx.setStatus(CommandContext.CMD_OK);
    } catch (Exception e) {
      e.printStackTrace();
    }
  }

}
