package ru.sibinco.smsx.engine.service.group.commands;

import ru.sibinco.smsx.engine.service.Command;
import ru.sibinco.smsx.engine.service.CommandExecutionException;

/**
 * User: artem
 * Date: 21.11.2008
 */
public class GroupEditGetProfileCmd extends Command {

  private String address;

  public String getAddress() {
    return address;
  }

  public void setAddress(String address) {
    this.address = address;
  }

  public static class Result {
    public boolean sendNotification;
    public boolean lockEdit;
  }

  public interface Receiver {
    public Result execute(GroupEditGetProfileCmd cmd) throws CommandExecutionException;
  }
}
