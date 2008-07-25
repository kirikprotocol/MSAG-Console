package ru.sibinco.smsx.engine.service.group.commands;

import ru.sibinco.smsx.engine.service.Command;
import ru.sibinco.smsx.engine.service.CommandExecutionException;

/**
 * User: artem
 * Date: 15.07.2008
 */

public class GroupSendCmd extends Command {

  public static final int ERR_UNKNOWN_OWNER = ERR_INT + 1;
  public static final int ERR_UNKNOWN_GROUP = ERR_INT + 2;

  // Input parameters
  private boolean storable;
  private String groupName;
  private String owner;
  private String message;
  private boolean express;
  private String imsi;
  private String mscAddress;

  public String getGroupName() {
    return groupName;
  }

  public GroupSendCmd setGroupName(String groupName) {
    this.groupName = groupName;
    return this;
  }

  public String getOwner() {
    return owner;
  }

  public GroupSendCmd setOwner(String owner) {
    this.owner = owner;
    return this;
  }

  public String getMessage() {
    return message;
  }

  public GroupSendCmd setMessage(String message) {
    this.message = message;
    return this;
  }

  public boolean isExpress() {
    return express;
  }

  public GroupSendCmd setExpress(boolean express) {
    this.express = express;
    return this;
  }

  public boolean isStorable() {
    return storable;
  }

  public GroupSendCmd setStorable(boolean storable) {
    this.storable = storable;
    return this;
  }

  public String getImsi() {
    return imsi;
  }

  public GroupSendCmd setImsi(String imsi) {
    this.imsi = imsi;
    return this;
  }

  public String getMscAddress() {
    return mscAddress;
  }

  public GroupSendCmd setMscAddress(String mscAddress) {
    this.mscAddress = mscAddress;
    return this;
  } 

  public interface Receiver {
    public long execute(GroupSendCmd cmd) throws CommandExecutionException;
  }
}
