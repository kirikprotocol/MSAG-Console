package ru.novosoft.smsc.admin.console.commands.infosme;

import ru.novosoft.smsc.admin.console.commands.CommandClass;
import ru.novosoft.smsc.admin.console.CommandContext;

import java.lang.reflect.Method;

/**
 * User: artem
 * Date: Aug 6, 2007
 */

public class InfosmeImportCommand extends CommandClass {

  private String file;
  private String splitByRegions;

  public void process(CommandContext ctx) {
    try {
      Class cmdClass = Class.forName("ru.novosoft.smsc.infosme.backend.commands.InfoSmeCommandsImpl");
      final Object cmd = cmdClass.newInstance();
      Method importFileMethod = null;
      try {
        importFileMethod = cmdClass.getMethod("importFile", new Class[]{CommandContext.class, String.class, Boolean.TYPE});
        importFileMethod.invoke(cmd, new Object[]{ctx, file, new Boolean(splitByRegions != null && splitByRegions.equalsIgnoreCase("true"))});
      } catch (NoSuchMethodException e) {
      }

      if (importFileMethod == null) {
        try {
          importFileMethod = cmdClass.getMethod("importFile", new Class[]{CommandContext.class, String.class});
          importFileMethod.invoke(cmd, new Object[]{ctx, file});
        } catch (NoSuchMethodException e) {
        }
      }

      if (importFileMethod == null) {
        ctx.setMessage("Can't find module InfoSme");
        ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
      }

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
    return "INFOSME_IMPORT";
  }

  public String getFile() {
    return file;
  }

  public void setFile(String file) {
    this.file = file;
  }

  public String getSplitByRegions() {
    return splitByRegions;
  }

  public void setSplitByRegions(String splitByRegions) {
    this.splitByRegions = splitByRegions;
  }
}
