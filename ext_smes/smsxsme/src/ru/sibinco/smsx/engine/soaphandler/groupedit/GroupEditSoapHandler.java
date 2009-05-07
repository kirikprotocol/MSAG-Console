package ru.sibinco.smsx.engine.soaphandler.groupedit;

import com.eyeline.sme.smpp.OutgoingObject;
import com.eyeline.sme.smpp.OutgoingQueue;
import com.eyeline.sme.smpp.ShutdownedException;
import com.eyeline.utils.config.ConfigException;
import com.eyeline.utils.config.xml.XmlConfig;
import org.apache.log4j.Category;
import ru.aurorisoft.smpp.Message;
import ru.sibinco.smsx.InitializationException;
import ru.sibinco.smsx.engine.service.CommandExecutionException;
import ru.sibinco.smsx.engine.service.Services;
import ru.sibinco.smsx.engine.service.group.commands.*;
import ru.sibinco.smsx.engine.service.group.datasource.DistrList;

import java.io.File;
import java.rmi.RemoteException;
import java.util.Collection;
import java.util.Properties;

/**
 * User: artem
 * Date: 11.07.2008
 */

class GroupEditSoapHandler implements GroupEdit {

  private static final Category log = Category.getInstance(GroupEditSoapHandler.class);

  private static final int RESULT_OK = 0;
  private static final int RESULT_SYSTEM_ERROR = -1;
  private static final int RESULT_INVALID_MEMBER = -2;
  private static final int RESULT_OWNER_NOT_EXISTS = -3;
  private static final int RESULT_INVALID_GROUPNAME = -4;
  private static final int RESULT_GROUP_ALREADY_EXISTS = -5;
  private static final int RESULT_GROUPS_LIMIT_EXCEEDED = -6;
  private static final int RESULT_GROUP_NOT_EXISTS = -7;
  private static final int RESULT_MEMBERS_COUNT_EXCEEDED = -8;
  private static final int RESULT_MEMBER_NOT_EXISTS = -9;
  private static final int RESULT_INVALID_OWNER = -10;
  private static final int RESULT_MEMBER_ALREADY_EXISTS = -11;
  private static final int RESULT_LOCKED_BY_OWNER = -12;

  private final OutgoingQueue outQueue;
  private final Properties texts;
  private final String oa;

  GroupEditSoapHandler(String configDir, OutgoingQueue outQueue) {
    this.outQueue = outQueue;
    XmlConfig config = new XmlConfig();
    try {
      config.load(new File(configDir, "soaphandlers/groupedit.xml"));
      texts = config.getSection("notifications").toProperties("");
      oa = config.getString("sourceAddress");
    } catch (ConfigException e) {
      throw new InitializationException(e);
    }
  }

  public GroupListResp groupList(String owner) throws RemoteException {
    if (log.isDebugEnabled())
      log.debug("GroupList req: owner=" + owner);

    long start  = System.currentTimeMillis();
    GroupListResp response = new GroupListResp();

    try {
      final GroupListCmd cmd = new GroupListCmd();
      cmd.setOwner(owner);
      Collection<DistrList> res = Services.getInstance().getGroupService().execute(cmd);

      String[] names = new String[res.size()];
      int i=0;
      for (DistrList l : res)
        names[i++] = l.getName();

      response.setGroups(names);
      response.setStatus(RESULT_OK);
    } catch (CommandExecutionException e) {
      response.setStatus(getStatus(e.getErrCode()));
    } catch (Throwable e) {
      response.setStatus(RESULT_SYSTEM_ERROR);
    } finally {
      if (log.isDebugEnabled())
        log.debug("Time=" + (System.currentTimeMillis() - start));
    }

    return response;
  }

  private GroupEditGetProfileCmd.Result getProfile(String owner) throws CommandExecutionException {
    GroupEditGetProfileCmd c = new GroupEditGetProfileCmd();
    c.setAddress(owner);
    return Services.getInstance().getGroupService().execute(c);
  }

  public int addGroup(String groupName, String owner) throws RemoteException {
    if (log.isDebugEnabled())
      log.debug("Group add req: group=" + groupName + "; owner=" + owner);

    long start = System.currentTimeMillis();
    try {
      GroupEditGetProfileCmd.Result profile = getProfile(owner);
      if (profile.lockEdit)
        return RESULT_LOCKED_BY_OWNER;

      GroupAddCmd c = new GroupAddCmd();
      c.setGroupName(groupName);
      c.setOwner(owner);
      Services.getInstance().getGroupService().execute(c);

      if (profile.sendNotification)
        sendMessage(owner, texts.getProperty("add.group").replace("{1}", groupName));

      return RESULT_OK;
    } catch (CommandExecutionException e) {
      return getStatus(e.getErrCode());
    } catch (Throwable e) {
      log.error(e, e);
      return RESULT_SYSTEM_ERROR;
    } finally {
      if (log.isDebugEnabled())
        log.debug("Time=" + (System.currentTimeMillis() - start));
    }
  }

  public int removeGroup(String groupName, String owner) throws RemoteException {
    if (log.isDebugEnabled())
      log.debug("Group remove req: group=" + groupName + "; owner=" + owner);
    long start = System.currentTimeMillis();
    try {
      GroupEditGetProfileCmd.Result profile = getProfile(owner);
      if (profile.lockEdit)
        return RESULT_LOCKED_BY_OWNER;

      GroupRemoveCmd c = new GroupRemoveCmd();
      c.setGroupName(groupName);
      c.setOwner(owner);
      Services.getInstance().getGroupService().execute(c);

      if (profile.sendNotification)
        sendMessage(owner, texts.getProperty("remove.group").replace("{1}", groupName));

      return RESULT_OK;
    } catch (CommandExecutionException e) {
      return getStatus(e.getErrCode());
    } catch (Throwable e) {
      log.error(e, e);
      return RESULT_SYSTEM_ERROR;
    } finally {
      if (log.isDebugEnabled())
        log.debug("Time=" + (System.currentTimeMillis() - start));
    }
  }

  public GroupInfoResp groupInfo(String groupName, String owner) throws RemoteException {
    if (log.isDebugEnabled())
      log.debug("Group info req: group=" + groupName + "; owner=" + owner);
    long start = System.currentTimeMillis();
    GroupInfoResp resp = new GroupInfoResp();
    resp.setName(groupName);
    resp.setOwner(owner);

    try {
      GroupInfoCmd c = new GroupInfoCmd();
      c.setGroupName(groupName);
      c.setOwner(owner);

      GroupInfo info = Services.getInstance().getGroupService().execute(c);
      String[] names = new String[info.getMembers().size()];
      int i=0;
      for (String member : info.getMembers())
        names[i++] = member;

      resp.setMembers(names);
      resp.setStatus(RESULT_OK);

    } catch (CommandExecutionException e) {
      resp.setStatus(getStatus(e.getErrCode()));
    } catch (Throwable e) {
      log.error(e, e);
      resp.setStatus(RESULT_SYSTEM_ERROR);
    } finally {
      if (log.isDebugEnabled())
        log.debug("Time=" + (System.currentTimeMillis() - start));
    }

    return resp;
  }

  public int addMember(String groupName, String owner, String member) throws RemoteException {
    if (log.isDebugEnabled())
      log.debug("Group add member req: groupName=" + groupName + "; owner=" + owner + "; member=" + member);
    long start = System.currentTimeMillis();
    try {
      GroupEditGetProfileCmd.Result profile = getProfile(owner);
      if (profile.lockEdit)
        return RESULT_LOCKED_BY_OWNER;

      GroupAddMemberCmd c = new GroupAddMemberCmd();
      c.setGroupName(groupName);
      c.setOwner(owner);
      c.setMember(prepareMsisdn(member));
      Services.getInstance().getGroupService().execute(c);

      if (profile.sendNotification)
        sendMessage(owner, texts.getProperty("add.member").replace("{1}", groupName).replace("{2}", member));

      return RESULT_OK;
    } catch (CommandExecutionException e) {
      return getStatus(e.getErrCode());
    } catch (Throwable e) {
      log.error(e, e);
      return RESULT_SYSTEM_ERROR;
    } finally {
      if (log.isDebugEnabled())
        log.debug("Time=" + (System.currentTimeMillis() - start));
    }
  }

  public int removeMember(String groupName, String owner, String member) throws RemoteException {
    if (log.isDebugEnabled())
      log.debug("Group remove member req: group=" + groupName + "; owner=" + owner + "; member=" + member);
    long start = System.currentTimeMillis();
    try {
      GroupEditGetProfileCmd.Result profile = getProfile(owner);
      if (profile.lockEdit)
        return RESULT_LOCKED_BY_OWNER;

      GroupRemoveMemberCmd c = new GroupRemoveMemberCmd();
      c.setGroupName(groupName);
      c.setOwner(owner);
      c.setMember(prepareMsisdn(member));
      Services.getInstance().getGroupService().execute(c);

      if (profile.sendNotification)
        sendMessage(owner, texts.getProperty("remove.member").replace("{1}", groupName).replace("{2}", member));

      return RESULT_OK;
    } catch (CommandExecutionException e) {
      return getStatus(e.getErrCode());
    } catch (Throwable e) {
      log.error(e, e);
      return RESULT_SYSTEM_ERROR;
    } finally {
      if (log.isDebugEnabled())
        log.debug("Time=" + (System.currentTimeMillis() - start));
    }
  }

  public int renameGroup(String groupName, String owner, String newName) throws RemoteException {
    if (log.isDebugEnabled())
      log.debug("Group rename req: group=" + groupName + "; owner=" + owner + "; newName=" + newName);
    long start = System.currentTimeMillis();
    try {
      GroupEditGetProfileCmd.Result profile = getProfile(owner);
      if (profile.lockEdit)
        return RESULT_LOCKED_BY_OWNER;

      GroupRenameCmd c = new GroupRenameCmd();
      c.setGroupName(groupName);
      c.setOwner(owner);
      c.setNewGroupName(newName);
      Services.getInstance().getGroupService().execute(c);

      if (profile.sendNotification)
        sendMessage(owner, texts.getProperty("rename.group").replace("{1}", groupName).replace("{2}", newName));

      return RESULT_OK;
    } catch (CommandExecutionException e) {
      return getStatus(e.getErrCode());
    } catch (Throwable e) {
      log.error(e, e);
      return RESULT_SYSTEM_ERROR;
    } finally {
      if (log.isDebugEnabled())
        log.debug("Time=" + (System.currentTimeMillis() - start));
    }
  }

  public int copyGroup(String groupName, String owner, String newName) throws RemoteException {
    if (log.isDebugEnabled())
      log.debug("Group copy req: group=" + groupName + "; owner=" + owner + "; newName=" + newName);
    long start = System.currentTimeMillis();
    try {
      GroupEditGetProfileCmd.Result profile = getProfile(owner);
      if (profile.lockEdit)
        return RESULT_LOCKED_BY_OWNER;

      GroupCopyCmd c = new GroupCopyCmd();
      c.setGroupName(groupName);
      c.setOwner(owner);
      c.setNewGroupName(newName);
      Services.getInstance().getGroupService().execute(c);

      if (profile.sendNotification)
        sendMessage(owner, texts.getProperty("copy.group").replace("{1}", groupName).replace("{2}", newName));

      return RESULT_OK;
    } catch (CommandExecutionException e) {
      return getStatus(e.getErrCode());
    } catch (Throwable e) {
      log.error(e, e);
      return RESULT_SYSTEM_ERROR;
    } finally {
      if (log.isDebugEnabled())
        log.debug("Time=" + (System.currentTimeMillis() - start));
    }
  }

  private static String prepareMsisdn(String msisdn) {
    if (msisdn == null || msisdn.length() == 0)
      return msisdn;

    if (msisdn.charAt(0) == '7')
      msisdn = '+' + msisdn;

    return msisdn;
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

  private static int getStatus(int errcode) {
    switch (errcode) {
      case GroupEditCommand.ERR_GROUP_ALREADY_EXISTS: return RESULT_GROUP_ALREADY_EXISTS;
      case GroupEditCommand.ERR_GROUP_NOT_EXISTS: return RESULT_GROUP_NOT_EXISTS;
      case GroupEditCommand.ERR_GROUPS_COUNT_EXCEEDED: return RESULT_GROUPS_LIMIT_EXCEEDED;
      case GroupEditCommand.ERR_INV_GROUP_NAME: return RESULT_INVALID_GROUPNAME;
      case GroupEditCommand.ERR_INV_MEMBER: return RESULT_INVALID_MEMBER;
      case GroupEditCommand.ERR_INV_OWNER: return RESULT_INVALID_OWNER;
      case GroupEditCommand.ERR_MEMBER_ALREADY_EXISTS: return RESULT_MEMBER_ALREADY_EXISTS;
      case GroupEditCommand.ERR_MEMBER_COUNT_EXCEEDED: return RESULT_MEMBERS_COUNT_EXCEEDED;
      case GroupEditCommand.ERR_OWNER_NOT_EXISTS: return RESULT_OWNER_NOT_EXISTS;
      case GroupEditCommand.ERR_MEMBER_NOT_EXISTS: return RESULT_MEMBER_NOT_EXISTS;
      case GroupEditCommand.ERR_LOCKED_BY_OWNER: return RESULT_LOCKED_BY_OWNER;
      default: return RESULT_SYSTEM_ERROR;
    }
  }
}
