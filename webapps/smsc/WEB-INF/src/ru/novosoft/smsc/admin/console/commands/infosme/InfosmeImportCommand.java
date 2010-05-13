package ru.novosoft.smsc.admin.console.commands.infosme;

import ru.novosoft.smsc.admin.console.commands.CommandClass;
import ru.novosoft.smsc.admin.console.CommandContext;

/**
 * User: artem
 * Date: Aug 6, 2007
 */

public class InfosmeImportCommand extends CommandClass {

  private String file;
  private String splitByRegions;

  public void process(CommandContext ctx) {
    try {
      final InfoSmeCommands cmd = (InfoSmeCommands)Class.forName("ru.novosoft.smsc.infosme.backend.commands.InfoSmeCommandsImpl").newInstance();
      cmd.importFile(ctx, file, splitByRegions != null && splitByRegions.equalsIgnoreCase("true"));
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
