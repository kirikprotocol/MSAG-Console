package ru.novosoft.smsc.admin.console.commands.infosme;

import ru.novosoft.smsc.admin.console.commands.CommandClass;
import ru.novosoft.smsc.admin.console.CommandContext;

import java.text.SimpleDateFormat;
import java.text.ParseException;
import java.util.Date;

/**
 * author: alkhal
 */
public class InfoSmeExportStatCommand extends CommandClass {

  private static final String dateTextFormat = "dd.MM.yyyy HH:mm";

  private SimpleDateFormat dateFormat = new SimpleDateFormat(dateTextFormat);

  private String taskName;

  private String fileName;

  private String startDate;

  private String startTime;


  public void process(CommandContext ctx) {

    if(taskName == null || taskName.length() == 0) {
      ctx.setMessage("Task name is empty");
      ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
      return;
    }
    if(fileName == null || fileName.length() == 0) {
      ctx.setMessage("File's name is empty");
      ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
      return;
    }
    Date date = null;
    if(startDate != null && startDate.length() > 0) {
      if(startTime == null || startTime.length() == 0) {
        ctx.setMessage("Illegal start date: '"+startDate+"' Date format is '"+dateTextFormat+'\'');
        ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
        return;
      }
      try {
        date = dateFormat.parse(startDate+' '+startTime);
      } catch (ParseException e) {
        ctx.setMessage("Illegal start date: '"+startDate+' '+startTime+"' Date format is '"+dateTextFormat+'\'');
        ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
        return;
      }
    }

    try {
      final InfoSmeCommands cmd = (InfoSmeCommands)Class.forName("ru.novosoft.smsc.infosme.backend.commands.InfoSmeCommandsImpl").newInstance();
      cmd.exportStat(ctx, taskName, fileName, date);
    } catch (ClassNotFoundException e) {
      e.printStackTrace();
      ctx.setMessage("Can't find module InfoSme");
      ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
    } catch (IllegalAccessException e) {
      e.printStackTrace();
      ctx.setMessage("Can't find module InfoSme");
      ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
    } catch (InstantiationException e) {
      e.printStackTrace();
      ctx.setMessage("Can't find module InfoSme");
      ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
    } catch (Throwable e) {
      e.printStackTrace();
      ctx.setMessage(e.getMessage());
      ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
    }
  }

  public String getId() {
    return "INFOSME_EXPORT_STAT";
  }

  public void setTaskName(String taskName) {
    this.taskName = taskName;
  }

  public void setFileName(String fileName) {
    this.fileName = fileName;
  }

  public void setStartDate(String startDate) {
    this.startDate = startDate;
  }

  public void setStartTime(String startTime) {
    this.startTime = startTime;
  }
}
