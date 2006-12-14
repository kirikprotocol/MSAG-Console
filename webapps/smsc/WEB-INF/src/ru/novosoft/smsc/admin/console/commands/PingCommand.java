package ru.novosoft.smsc.admin.console.commands;

import ru.novosoft.smsc.admin.console.CommandContext;

/**
 * User: artem
 * Date: 14.12.2006
 */
public class PingCommand extends CommandClass{
  public static final String ID = "PING";

  public void process(CommandContext ctx) {
    ctx.setMessage("");
  }

  public String getId() {
    return ID;
  }
}
