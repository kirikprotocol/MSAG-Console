package ru.novosoft.smsc.admin.dl.soap;

import ru.novosoft.smsc.jsp.SMSCAppContextImpl;
import ru.novosoft.smsc.jsp.util.tables.impl.dl.DlFilter;
import ru.novosoft.smsc.admin.dl.DistributionListAdmin;
import ru.novosoft.smsc.admin.dl.DistributionList;
import ru.novosoft.smsc.admin.dl.Principal;
import ru.novosoft.smsc.admin.dl.exceptions.*;
import ru.novosoft.smsc.admin.AdminException;

import java.rmi.RemoteException;
import java.util.List;
import java.util.Iterator;

import org.apache.log4j.Category;

/**
 * User: artem
 * Date: 11.07.2008
 */

public class GroupEditImpl implements GroupEdit {

  private static final Category log = Category.getInstance(GroupEditImpl.class);

  private static final int MAX_GROUP_NAME_LEN=18;

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

  private final DistributionListAdmin admin;

  public GroupEditImpl() {
    admin = SMSCAppContextImpl.getInstance().getSmsc().getDistributionListAdmin();
  }

  private static boolean isEmpty(String str) {
    return str == null || str.trim().length() == 0;
  }

  public GroupListResp groupList(String owner) throws RemoteException {
    GroupListResp response = new GroupListResp();

    if (!isEmpty(owner)) {
      final DlFilter filter = new DlFilter();
      filter.setOwners(new String[]{owner});

      try {
        List res = admin.list(filter);
        String[] names = new String[res.size()];
        for (int i=0; i<names.length; i++)
          names[i] = getGroupShortName(((DistributionList)res.get(i)).getName());

        response.setGroups(names);
        response.setStatus(RESULT_OK);
      } catch (Exception e) {
        log.error(e,e);
        response.setStatus(getStatus(e));
      }

    } else
      response.setStatus(RESULT_INVALID_OWNER);

    return response;
  }

  private static String getGroupShortName(String fullName) {
    int i = fullName.indexOf('/');
    return i > 0 ? fullName.substring(i+1) : fullName;
  }

  private static String getGroupFullName(String groupShortName, String owner) {
    return owner.replaceFirst("\\+","") + '/' + groupShortName;
  }

  public int addGroup(String groupName, String owner) throws RemoteException {
    if (isEmpty(groupName) || groupName.length() > MAX_GROUP_NAME_LEN)
      return RESULT_INVALID_GROUPNAME;
    if (isEmpty(owner))
      return RESULT_INVALID_OWNER;

    for (int i=0; i<2; i++) {
      try {
        final DistributionList list = new DistributionList(getGroupFullName(groupName, owner), owner, 50);
        admin.addDistributionList(list);
        return RESULT_OK;

      } catch (PrincipalNotExistsException e) { // If principal does not exists, create it
        log.debug("Principal " + owner + " does not exists. Try to create it.");

        final Principal p = new Principal(owner, 50, 50);
        try {
          admin.addPrincipal(p);
        } catch (AdminException e1) {
          log.error("Principal creation error: ",e);
          return getStatus(e);
        }

      } catch (AdminException e) {
        log.error(e,e);
        return getStatus(e);
      }
    }
    return RESULT_SYSTEM_ERROR;
  }

  public int removeGroup(String groupName, String owner) throws RemoteException {
    if (isEmpty(groupName))
      return RESULT_INVALID_GROUPNAME;
    if (isEmpty(owner))
      return RESULT_INVALID_OWNER;

    try {
      admin.deleteDistributionList(getGroupFullName(groupName, owner));
      return RESULT_OK;
    } catch (Exception e) {
      log.error(e,e);
      return getStatus(e);
    }
  }

  public GroupInfoResp groupInfo(String groupName, String owner) throws RemoteException {
    GroupInfoResp resp = new GroupInfoResp();
    if (isEmpty(groupName)) {
      resp.setStatus(RESULT_INVALID_GROUPNAME);
    } else if (isEmpty(owner)) {
      resp.setStatus(RESULT_INVALID_OWNER);
    } else {
      try {
        List members = admin.members(getGroupFullName(groupName, owner));
        if (members != null) {
          String[] list = new String[members.size()];
          for (int i=0; i<members.size(); i++)
            list[i] = (String)members.get(i);

          resp.setMembers(list);
          resp.setName(groupName);
          resp.setOwner(owner);
          resp.setStatus(RESULT_OK);
        } else
          resp.setStatus(RESULT_GROUP_NOT_EXISTS);

      } catch (Exception e) {
        log.error(e,e);
        resp.setStatus(getStatus(e));
      }
    }
    return resp;
  }

  public int addMember(String groupName, String owner, String member) throws RemoteException {
    if (isEmpty(groupName))
      return RESULT_INVALID_GROUPNAME;
    if (isEmpty(owner))
      return RESULT_INVALID_OWNER;
    if (isEmpty(member))
      return RESULT_INVALID_MEMBER;

    try {
      admin.addMember(getGroupFullName(groupName, owner), member);
      return RESULT_OK;
    } catch (Exception e) {
      log.error(e,e);
      return getStatus(e);
    }
  }

  public int removeMember(String groupName, String owner, String member) throws RemoteException {
    if (isEmpty(groupName))
      return RESULT_INVALID_GROUPNAME;
    if (isEmpty(owner))
      return RESULT_INVALID_OWNER;
    if (isEmpty(member))
      return RESULT_INVALID_MEMBER;

    try {
      admin.deleteMember(getGroupFullName(groupName, owner), member);
      return RESULT_OK;
    } catch (Exception e) {
      log.error(e,e);
      return getStatus(e);
    }
  }

  public int renameGroup(String groupName, String owner, String newName) throws RemoteException {
    if (isEmpty(groupName))
      return RESULT_INVALID_GROUPNAME;
    if (isEmpty(owner))
      return RESULT_INVALID_OWNER;
    if (isEmpty(newName) || newName.length() > MAX_GROUP_NAME_LEN)
      return RESULT_INVALID_GROUPNAME;

    String oldGroupName = getGroupFullName(groupName, owner);
    String newGroupName = getGroupFullName(newName, owner);
    try {
      DistributionList l = admin.getDistributionList(oldGroupName);
      List members = admin.members(oldGroupName);

      DistributionList newList = new DistributionList(newGroupName, owner, l.getMaxElements());
      admin.addDistributionList(newList);

      for (Iterator iter = members.iterator(); iter.hasNext();)
        admin.addMember(newGroupName, (String)iter.next());

      admin.deleteDistributionList(l.getName());
    } catch (Exception e) {
      log.error(e,e);
      try {
        admin.deleteDistributionList(newGroupName);
      } catch (AdminException e1) {
      }
      return getStatus(e);
    }

    return 0;
  }


  private static int getStatus(Exception e) {
    if (e instanceof ListAlreadyExistsException)
      return RESULT_GROUP_ALREADY_EXISTS;
    else if (e instanceof ListNotExistsException)
      return RESULT_GROUP_NOT_EXISTS;
    else if (e instanceof OwnerNotExistsException)
      return RESULT_OWNER_NOT_EXISTS;
    else if (e instanceof ListsCountExceededException)
      return RESULT_GROUPS_LIMIT_EXCEEDED;
    else if (e instanceof MembersCountExceededException)
      return RESULT_MEMBERS_COUNT_EXCEEDED;
    else if (e instanceof MemberNotExistsException)
      return RESULT_MEMBER_NOT_EXISTS;
    else if (e instanceof MemberAlreadyExistsException)
      return RESULT_MEMBER_ALREADY_EXISTS;
    else
      return RESULT_SYSTEM_ERROR;
  }
}
