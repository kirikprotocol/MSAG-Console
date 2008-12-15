package ru.sibinco.smsx.engine.service.group.commands;

import ru.sibinco.smsx.engine.service.Command;

/**
 * User: artem
 * Date: 25.07.2008
 */

public class GroupEditCommand extends Command {

  public static final int ERR_INV_GROUP_NAME            = ERR_INT + 1;
  public static final int ERR_INV_OWNER                 = ERR_INT + 2;
  public static final int ERR_GROUP_ALREADY_EXISTS      = ERR_INT + 3;
  public static final int ERR_GROUPS_COUNT_EXCEEDED     = ERR_INT + 4;
  public static final int ERR_GROUP_NOT_EXISTS          = ERR_INT + 5;
  public static final int ERR_MEMBER_ALREADY_EXISTS     = ERR_INT + 6;
  public static final int ERR_MEMBER_COUNT_EXCEEDED     = ERR_INT + 7;
  public static final int ERR_OWNER_NOT_EXISTS          = ERR_INT + 8;
  public static final int ERR_INV_MEMBER                = ERR_INT + 9;
  public static final int ERR_MEMBER_NOT_EXISTS         = ERR_INT + 10;
  public static final int ERR_LOCKED_BY_OWNER           = ERR_INT + 11;
  public static final int ERR_SUBMITTER_NOT_EXISTS      = ERR_INT + 12;
  public static final int ERR_SUBMITTER_ALREADY_EXISTS  = ERR_INT + 13;
  public static final int ERR_SUBMITTERS_COUNT_EXCEEDED = ERR_INT + 14;
  public static final int ERR_PRINCIPAL_ALREADY_EXISTS  = ERR_INT + 15;
  public static final int ERR_INVALID_PRINCIPAL_ADDRESS = ERR_INT + 16;
  public static final int ERR_PRINCIPAL_NOT_EXISTS      = ERR_INT + 17;
  public static final int ERR_INV_SUBMITTER             = ERR_INT + 18;

  protected String groupName;
  protected String owner;

  public String getGroupName() {
    return groupName;
  }

  public void setGroupName(String groupName) {
    this.groupName = groupName;
  }

  public String getOwner() {
    return owner;
  }

  public void setOwner(String owner) {
    this.owner = owner;
  }

  public String toString() {
    return "Req: name="+ groupName + "; owner="+ owner;
  }
}
