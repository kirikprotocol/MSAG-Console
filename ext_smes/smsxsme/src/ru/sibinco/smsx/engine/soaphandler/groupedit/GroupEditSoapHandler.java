package ru.sibinco.smsx.engine.soaphandler.groupedit;

import java.rmi.RemoteException;
import java.util.List;
import java.util.Iterator;

import org.apache.log4j.Category;
import ru.sibinco.smsx.engine.service.CommandExecutionException;
import ru.sibinco.smsx.engine.service.Services;
import ru.sibinco.smsx.engine.service.group.commands.*;
import ru.sibinco.smsc.utils.admin.dl.DistributionList;

/**
 * User: artem
 * Date: 11.07.2008
 */

class GroupEditSoapHandler implements GroupEdit {

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

  GroupEditSoapHandler(String configDir) {

  }

  public GroupListResp groupList(String owner) throws RemoteException {
    GroupListResp response = new GroupListResp();

    try {
      final GroupListCmd cmd = new GroupListCmd();
      cmd.setOwner(owner);
      List<DistributionList> res = Services.getInstance().getGroupService().execute(cmd);

      String[] names = new String[res.size()];
      int i=0;
      for (DistributionList l : res)
        names[i++] = l.getName();

      response.setGroups(names);
      response.setStatus(RESULT_OK);
    } catch (CommandExecutionException e) {
      e.printStackTrace();
      response.setStatus(getStatus(e.getErrCode()));
    }

    return response;
  }

  public int addGroup(String groupName, String owner) throws RemoteException {
    try {
      GroupAddCmd c = new GroupAddCmd();
      c.setGroupName(groupName);
      c.setOwner(owner);
      Services.getInstance().getGroupService().execute(c);
      return RESULT_OK;
    } catch (CommandExecutionException e) {
      return getStatus(e.getErrCode());
    }
  }

  public int removeGroup(String groupName, String owner) throws RemoteException {
    try {
      GroupRemoveCmd c = new GroupRemoveCmd();
      c.setGroupName(groupName);
      c.setOwner(owner);
      Services.getInstance().getGroupService().execute(c);
      return RESULT_OK;
    } catch (CommandExecutionException e) {
      return getStatus(e.getErrCode());
    }
  }

  public GroupInfoResp groupInfo(String groupName, String owner) throws RemoteException {
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
    }

    return resp;
  }

  public int addMember(String groupName, String owner, String member) throws RemoteException {
    try {
      GroupAddMemberCmd c = new GroupAddMemberCmd();
      c.setGroupName(groupName);
      c.setOwner(owner);
      c.setMember(member);
      Services.getInstance().getGroupService().execute(c);
      return RESULT_OK;
    } catch (CommandExecutionException e) {
      return getStatus(e.getErrCode());
    }
  }

  public int removeMember(String groupName, String owner, String member) throws RemoteException {
    try {
      GroupRemoveMemberCmd c = new GroupRemoveMemberCmd();
      c.setGroupName(groupName);
      c.setOwner(owner);
      c.setMember(member);
      Services.getInstance().getGroupService().execute(c);
      return RESULT_OK;
    } catch (CommandExecutionException e) {
      return getStatus(e.getErrCode());
    }
  }

  public int renameGroup(String groupName, String owner, String newName) throws RemoteException {
    try {
      GroupRenameCmd c = new GroupRenameCmd();
      c.setGroupName(groupName);
      c.setOwner(owner);
      c.setNewGroupName(newName);
      Services.getInstance().getGroupService().execute(c);
      return RESULT_OK;
    } catch (CommandExecutionException e) {
      return getStatus(e.getErrCode());
    }
  }

  private static int getStatus(int errcode) {
    switch (errcode) {
      case GroupCommand.ERR_GROUP_ALREADY_EXISTS: return RESULT_GROUP_ALREADY_EXISTS;
      case GroupCommand.ERR_GROUP_NOT_EXISTS: return RESULT_GROUP_NOT_EXISTS;
      case GroupCommand.ERR_GROUPS_COUNT_EXCEEDED: return RESULT_GROUPS_LIMIT_EXCEEDED;
      case GroupCommand.ERR_INV_GROUP_NAME: return RESULT_INVALID_GROUPNAME;
      case GroupCommand.ERR_INV_MEMBER: return RESULT_INVALID_MEMBER;
      case GroupCommand.ERR_INV_OWNER: return RESULT_INVALID_OWNER;
      case GroupCommand.ERR_MEMBER_ALREADY_EXISTS: return RESULT_MEMBER_ALREADY_EXISTS;
      case GroupCommand.ERR_MEMBER_COUNT_EXCEEDED: return RESULT_MEMBERS_COUNT_EXCEEDED;
      case GroupCommand.ERR_OWNER_NOT_EXISTS: return RESULT_OWNER_NOT_EXISTS;
      case GroupCommand.ERR_MEMBER_NOT_EXISTS: return RESULT_MEMBER_NOT_EXISTS;
      default: return RESULT_SYSTEM_ERROR;
    }
  }
}
