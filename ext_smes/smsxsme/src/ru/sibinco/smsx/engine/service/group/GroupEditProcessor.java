package ru.sibinco.smsx.engine.service.group;

import ru.sibinco.smsx.engine.service.group.commands.*;
import ru.sibinco.smsx.engine.service.group.datasource.GroupEditDataSource;
import ru.sibinco.smsx.engine.service.group.datasource.GroupEditProfile;
import ru.sibinco.smsx.engine.service.CommandExecutionException;
import ru.sibinco.smsx.engine.service.Command;
import ru.sibinco.smsx.utils.DataSourceException;
import ru.sibinco.smsc.utils.admin.dl.DistributionList;
import ru.sibinco.smsc.utils.admin.dl.Principal;
import ru.sibinco.smsc.utils.admin.dl.DistributionListManager;
import ru.sibinco.smsc.utils.admin.dl.exceptions.*;
import ru.sibinco.smsc.utils.admin.AdminException;
import ru.aurorisoft.smpp.Message;

import java.util.List;
import java.util.Properties;

import org.apache.log4j.Category;
import com.eyeline.sme.smpp.OutgoingQueue;
import com.eyeline.sme.smpp.OutgoingObject;
import com.eyeline.sme.smpp.ShutdownedException;

/**
 * User: artem
 * Date: 25.07.2008
 */

class GroupEditProcessor implements GroupAddCmd.Receiver,
                                GroupRemoveCmd.Receiver,
                                GroupRenameCmd.Receiver,
                                GroupCopyCmd.Receiver,
                                GroupAddMemberCmd.Receiver,
                                GroupRemoveMemberCmd.Receiver,
                                GroupInfoCmd.Receiver,
                                GroupListCmd.Receiver,
                                GroupEditAlterProfileCmd.Receiver {

  private static final Category log = Category.getInstance(GroupEditProcessor.class);

  private static final int MAX_GROUP_NAME_LEN = 18;

  private final OutgoingQueue outQueue;
  private final DistributionListManager dlmanager;
  private final GroupEditDataSource ds;
  private final String oa;
  private final Properties notifications;

  GroupEditProcessor(OutgoingQueue outQueue, DistributionListManager dlmanager, GroupEditDataSource ds, String oa, Properties notifications) {
    this.outQueue = outQueue;
    this.dlmanager = dlmanager;
    this.ds = ds;
    this.oa = oa;
    this.notifications = notifications;
  }

  private static void checkGroup(String groupName, String owner) throws CommandExecutionException {
    if (isEmpty(groupName) || groupName.length() > MAX_GROUP_NAME_LEN)
      throw new CommandExecutionException("Invalid group name", GroupEditCommand.ERR_INV_GROUP_NAME);

    if (isEmpty(owner))
      throw new CommandExecutionException("Invalid owner address", GroupEditCommand.ERR_INV_OWNER);
  }

  private GroupEditProfile loadProfile(String address) throws CommandExecutionException {
    try {
      GroupEditProfile profile = ds.loadProfile(address);
      if (profile != null && profile.isLockGroupEdit())
        throw new CommandExecutionException("Operation locked by owner", GroupEditCommand.ERR_LOCKED_BY_OWNER);
      return profile;
    } catch (DataSourceException e) {
      throw new CommandExecutionException(e.getMessage(), GroupEditCommand.ERR_SYS_ERROR);
    }
  }

  public void execute(GroupAddCmd cmd) throws CommandExecutionException {

    checkGroup(cmd.getGroupName(), cmd.getOwner());
    GroupEditProfile profile = loadProfile(cmd.getOwner());

    final DistributionList list = new DistributionList(cmd.getGroupName(), cmd.getOwner(), 50);
    for (int i=0; i<2; i++) {
      try {
        dlmanager.addDistributionList(list);
        if (profile == null || profile.isSendSmsNotification())
          sendMessage(cmd.getOwner(), notifications.getProperty("add.group").replace("{1}", '"' + cmd.getGroupName() + '"'));
        return;

      } catch (PrincipalNotExistsException e) { // If principal does not exists, create it
        if (log.isDebugEnabled())
          log.debug("Principal " + cmd.getOwner() + " does not exists. Try to create it.");

        final Principal p = new Principal(cmd.getOwner(), 50, 50);
        try {
          dlmanager.addPrincipal(p);
        } catch (AdminException e1) {
          log.error("Principal creation error: ",e);
          throw new CommandExecutionException("Can't create principal", GroupAddCmd.ERR_SYS_ERROR);
        }

      } catch (AdminException e) {
        catchGroupException(e, cmd);
      }
    }
    throw new CommandExecutionException("Can't create group", GroupAddCmd.ERR_SYS_ERROR);
  }

  public void execute(GroupRemoveCmd cmd) throws CommandExecutionException {
    checkGroup(cmd.getGroupName(), cmd.getOwner());
    GroupEditProfile profile = loadProfile(cmd.getOwner());

    try {
      dlmanager.deleteDistributionList(cmd.getGroupName(), cmd.getOwner());
      if (profile == null || profile.isSendSmsNotification())
          sendMessage(cmd.getOwner(), notifications.getProperty("remove.group").replace("{1}", '"' + cmd.getGroupName() + '"'));
    } catch (AdminException e) {
      catchGroupException(e, cmd);
    }
  }

  public void execute(GroupRenameCmd cmd) throws CommandExecutionException {
    checkGroup(cmd.getGroupName(), cmd.getOwner());
    checkGroup(cmd.getNewGroupName(), cmd.getOwner());
    
    GroupEditProfile profile = loadProfile(cmd.getOwner());

    try {
      dlmanager.renameDistributionList(cmd.getGroupName(), cmd.getOwner(), cmd.getNewGroupName());
      if (profile == null || profile.isSendSmsNotification())
        sendMessage(cmd.getOwner(), notifications.getProperty("rename.group").replace("{1}", '"' + cmd.getGroupName() + '"').replace("{2}", '"' + cmd.getNewGroupName() + '"'));
    } catch (AdminException e) {
      catchGroupException(e, cmd);
    }
  }

  public void execute(GroupCopyCmd cmd) throws CommandExecutionException {
    checkGroup(cmd.getGroupName(), cmd.getOwner());
    checkGroup(cmd.getNewGroupName(), cmd.getOwner());

    GroupEditProfile profile = loadProfile(cmd.getOwner());

    try {
      dlmanager.copyDistributionList(cmd.getGroupName(), cmd.getOwner(), cmd.getNewGroupName());
      if (profile == null || profile.isSendSmsNotification())
        sendMessage(cmd.getOwner(), notifications.getProperty("copy.group").replace("{1}", '"' + cmd.getGroupName() + '"').replace("{2}", '"' + cmd.getNewGroupName() + '"'));
    } catch (AdminException e) {
      catchGroupException(e, cmd);
    }
  }

  public void execute(GroupAddMemberCmd cmd) throws CommandExecutionException {
    checkGroup(cmd.getGroupName(), cmd.getOwner());
    if (isEmpty(cmd.getMember()))
      throw new CommandExecutionException("Invalid member msisdn", GroupAddMemberCmd.ERR_INV_MEMBER);

    GroupEditProfile profile = loadProfile(cmd.getOwner());

    try {
      dlmanager.addMember(cmd.getGroupName(), cmd.getOwner(), cmd.getMember());
      if (profile == null || profile.isSendSmsNotification())
          sendMessage(cmd.getOwner(), notifications.getProperty("add.member").replace("{1}", '"' + cmd.getGroupName() + '"').replace("{2}", '"' + cmd.getMember() + '"'));
    } catch (AdminException e) {
      catchGroupException(e, cmd);
    }
  }

  public void execute(GroupRemoveMemberCmd cmd) throws CommandExecutionException {
    checkGroup(cmd.getGroupName(), cmd.getOwner());
    if (isEmpty(cmd.getMember()))
      throw new CommandExecutionException("Invalid member msisdn", GroupAddMemberCmd.ERR_INV_MEMBER);

    GroupEditProfile profile = loadProfile(cmd.getOwner());

    try {
      dlmanager.deleteMember(cmd.getGroupName(), cmd.getOwner(), cmd.getMember());
      if (profile == null || profile.isSendSmsNotification())
          sendMessage(cmd.getOwner(), notifications.getProperty("remove.member").replace("{1}", '"' + cmd.getGroupName() + '"').replace("{2}", '"' + cmd.getMember() + '"'));
    } catch (AdminException e) {
      catchGroupException(e, cmd);
    }
  }

  public GroupInfo execute(GroupInfoCmd cmd) throws CommandExecutionException {
    checkGroup(cmd.getGroupName(), cmd.getOwner());

    try {
      List<String> members = dlmanager.members(cmd.getGroupName(), cmd.getOwner());
      return new GroupInfo(members);
    } catch (AdminException e) {
      catchGroupException(e, cmd);
    }
    return null;
  }

  public List<DistributionList> execute(GroupListCmd cmd) throws CommandExecutionException {
    if (isEmpty(cmd.getOwner()))
      throw new CommandExecutionException("Invalid owner msisdn", GroupEditCommand.ERR_INV_OWNER);

    try {
      return dlmanager.list(cmd.getOwner());
    } catch (AdminException e) {
      catchGroupException(e, cmd);
    }
    return null;
  }

  public void execute(GroupEditAlterProfileCmd cmd) throws CommandExecutionException {
    final GroupEditProfile p = new GroupEditProfile(cmd.getAddress());
    p.setLockGroupEdit(cmd.getLockGroupEdit());
    p.setSendSmsNotification(cmd.getSendNotifications());

    try {
      ds.saveProfile(p);      
    } catch (DataSourceException e) {
      throw new CommandExecutionException(e.getMessage(), Command.ERR_SYS_ERROR);
    }
  }

  private void sendMessage(String da, String text) {
    Message m = new Message();
    m.setSourceAddress(oa);
    m.setDestinationAddress(da);
    m.setMessageString(text);
    m.setConnectionName("smsx");

    OutgoingObject o = new OutgoingObject();
    o.setMessage(m);

    try {
      outQueue.offer(o);
    } catch (ShutdownedException e) {
      log.error("Can't send message", e);
    }
  }

  private static void catchGroupException(AdminException ex, GroupEditCommand cmd) throws CommandExecutionException {
    try {
      log.error("Cmd execution failed: owner=" + cmd.getOwner() + "; dl=" + cmd.getGroupName());
      throw ex;
    } catch (ListNotExistsException e) {
      throw new CommandExecutionException(e.getMessage(), GroupEditCommand.ERR_GROUP_NOT_EXISTS);
    } catch (MemberAlreadyExistsException e) {
      throw new CommandExecutionException(e.getMessage(), GroupEditCommand.ERR_MEMBER_ALREADY_EXISTS);
    } catch (MembersCountExceededException e) {
      throw new CommandExecutionException(e.getMessage(), GroupEditCommand.ERR_MEMBER_COUNT_EXCEEDED);
    } catch (PrincipalNotExistsException e) {
      throw new CommandExecutionException(e.getMessage(), GroupEditCommand.ERR_OWNER_NOT_EXISTS);
    } catch (ListAlreadyExistsException e) {
      throw new CommandExecutionException(e.getMessage(), GroupEditCommand.ERR_GROUP_ALREADY_EXISTS);
    } catch (ListsCountExceededException e) {
      throw new CommandExecutionException(e.getMessage(), GroupEditCommand.ERR_GROUPS_COUNT_EXCEEDED);
    } catch (MemberNotExistsException e) {
      throw new CommandExecutionException(e.getMessage(), GroupEditCommand.ERR_MEMBER_NOT_EXISTS);
    } catch (AdminException e) {
      throw new CommandExecutionException(e.getMessage(), GroupAddMemberCmd.ERR_SYS_ERROR);
    }
  }

  private static boolean isEmpty(String str) {
    return str == null || str.trim().length() == 0;
  }
}
