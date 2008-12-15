package ru.sibinco.smsx.engine.service.group;

import org.apache.log4j.Category;
import ru.sibinco.smsx.engine.service.Command;
import ru.sibinco.smsx.engine.service.CommandExecutionException;
import ru.sibinco.smsx.engine.service.group.commands.*;
import ru.sibinco.smsx.engine.service.group.datasource.*;
import ru.sibinco.smsx.utils.DataSourceException;
import ru.sibinco.smsx.utils.OperatorsList;

import java.util.Collection;

import com.eyeline.utils.config.xml.XmlConfigSection;
import com.eyeline.utils.config.ConfigException;

/**
 * User: artem
 * Date: 25.07.2008
 */

class GroupEditProcessor implements GroupAddCmd.Receiver,
                                GroupRemoveCmd.Receiver,
                                GroupRenameCmd.Receiver,
                                GroupAlterCmd.Receiver,
                                GroupCopyCmd.Receiver,
                                GroupAddMemberCmd.Receiver,
                                GroupRemoveMemberCmd.Receiver,
                                GroupInfoCmd.Receiver,
                                GroupListCmd.Receiver,
                                GroupAddSubmitterCmd.Receiver,
                                GroupRemoveSubmitterCmd.Receiver,
                                PrincipalAddCmd.Receiver,
                                PrincipalRemoveCmd.Receiver,
                                PrincipalListCmd.Receiver,
                                PrincipalGetCmd.Receiver,
                                PrincipalAlterCmd.Receiver {

  private static final Category log = Category.getInstance("GROUP");


  private static final int MAX_GROUP_NAME_LEN = 18;

  private final DistrListDataSource listsDS;
  private final PrincipalDataSource principalsDS;
  private final OperatorsList operators;
  private final int maxElements;
  private final int maxLists;

  GroupEditProcessor(XmlConfigSection sec, DistrListDataSource listsDS, PrincipalDataSource principalsDS, OperatorsList operators) throws ConfigException {
    this.listsDS = listsDS;
    this.principalsDS = principalsDS;
    this.operators = operators;
    this.maxElements = sec.getInt("max.elements");
    this.maxLists = sec.getInt("max.lists");
  }

  private static void checkGroup(String groupName) throws CommandExecutionException {
    if (isEmpty(groupName) || groupName.length() > MAX_GROUP_NAME_LEN)
      throw new CommandExecutionException("Invalid group name: " + groupName, GroupEditCommand.ERR_INV_GROUP_NAME);    
  }

  private static boolean checkMsisdn(String msisdn) {
    if (msisdn.charAt(0) == '+')
      msisdn = msisdn.substring(1);
    try {
      Long.parseLong(msisdn);
    } catch (NumberFormatException e) {
      return false;
    }
    return true;
  }

  private static boolean isEmpty(String str) {
    return str == null || str.trim().length() == 0;
  }

  // GROUPS ------------------------------------------------------------------------------------------------------------

  public void execute(GroupAddCmd cmd) throws CommandExecutionException {
    String owner = cmd.getOwner();
    String name = cmd.getGroupName();

    if (log.isDebugEnabled())
      log.debug(cmd);

    checkGroup(name);

    if (owner!=null && !checkMsisdn(owner))
      throw new CommandExecutionException("Invalid owner: " + owner, GroupEditCommand.ERR_INV_OWNER);

    try {
      Principal p = null;
      if (owner != null) {
        p = principalsDS.getPrincipal(owner);
        if (p == null) {
          p = new Principal(owner);
          p.setMaxLists(maxLists);
          p.setMaxMembersPerList(maxElements);
          principalsDS.addPrincipal(p);
        }
      }

      if (p != null && listsDS.getDistrLists(owner).size() >= p.getMaxLists())
        throw new CommandExecutionException("Groups count exceeded for " + owner, GroupEditCommand.ERR_GROUPS_COUNT_EXCEEDED);

      if (listsDS.containsDistrList(cmd.getGroupName(), owner))
        throw new CommandExecutionException("Group: name=" + name + "; owner=" + owner + " already exists.", GroupEditCommand.ERR_GROUP_ALREADY_EXISTS);

      int maxElements;
      if (cmd.getMaxElements() > 0)
        maxElements = cmd.getMaxElements();
      else
        maxElements = p == null ? Integer.MAX_VALUE : p.getMaxMembersPerList();

      DistrList dl = listsDS.createDistrList(name, owner, maxElements);
      if (owner != null)
        dl.addSubmitter(cmd.getOwner());

    } catch (DataSourceException e) {
      log.error(e,e);
      throw new CommandExecutionException(e.getMessage(), GroupEditCommand.ERR_SYS_ERROR);
    }
  }

  public void execute(GroupRemoveCmd cmd) throws CommandExecutionException {
    String owner = cmd.getOwner();
    String name = cmd.getGroupName();

    if (log.isDebugEnabled())
      log.debug(cmd);

    checkGroup(name);

    if (owner != null && !checkMsisdn(owner))
      throw new CommandExecutionException("Invalid owner: " + owner, GroupEditCommand.ERR_INV_OWNER);

    try {
      if (!listsDS.containsDistrList(name, owner))
        throw new CommandExecutionException("Group: name=" + name + "; owner=" + owner + " does not exists", GroupEditCommand.ERR_GROUP_NOT_EXISTS);

      listsDS.removeDistrList(name, owner);

    } catch (DataSourceException e) {
      log.error(e,e);
      throw new CommandExecutionException(e.getMessage(), GroupEditCommand.ERR_SYS_ERROR);
    }
  }

  private void copyGroup(String source, String dest, String owner) throws CommandExecutionException {
    checkGroup(source);
    checkGroup(dest);

    if (owner != null && !checkMsisdn(owner))
      throw new CommandExecutionException("Invalid owner: " + owner, GroupEditCommand.ERR_INV_OWNER);

    try {
      DistrList sourceDl = listsDS.getDistrList(source, owner);
      if (sourceDl == null)
        throw new CommandExecutionException("Group " + source + " does not exists", GroupEditCommand.ERR_GROUP_NOT_EXISTS);

      if (listsDS.containsDistrList(dest, owner))
        throw new CommandExecutionException("Group " + dest + " already exists", GroupEditCommand.ERR_GROUP_ALREADY_EXISTS);

      final DistrList destDl = listsDS.createDistrList(dest, owner, sourceDl.getMaxElements());
      destDl.copyFrom(sourceDl);

    } catch (DataSourceException e) {
      log.error(e,e);
      throw new CommandExecutionException(e.getMessage(), GroupEditCommand.ERR_SYS_ERROR);
    }
  }

  public void execute(GroupRenameCmd cmd) throws CommandExecutionException {
    if (log.isDebugEnabled())
      log.debug(cmd);

    copyGroup(cmd.getGroupName(), cmd.getNewGroupName(), cmd.getOwner());
    try {
      listsDS.removeDistrList(cmd.getGroupName(), cmd.getOwner());
    } catch (DataSourceException e) {
      log.error(e,e);
      throw new CommandExecutionException(e.getMessage(), GroupEditCommand.ERR_SYS_ERROR);
    }
  }

  public void execute(GroupCopyCmd cmd) throws CommandExecutionException {
    if (log.isDebugEnabled())
      log.debug(cmd);
    copyGroup(cmd.getGroupName(), cmd.getNewGroupName(), cmd.getOwner());
  }

  public void execute(GroupAlterCmd cmd) throws CommandExecutionException {
    if (log.isDebugEnabled())
      log.debug(cmd);

    String owner = cmd.getOwner();
    String name = cmd.getGroupName();

    checkGroup(name);

    if (owner != null && !checkMsisdn(owner))
      throw new CommandExecutionException("Invalid owner: " + owner, GroupEditCommand.ERR_INV_OWNER);

    try {
      DistrList dl = listsDS.getDistrList(name, owner);
      if (dl == null)
        throw new CommandExecutionException("Group: name=" + name + "; owner=" + owner + " does not exists", GroupEditCommand.ERR_GROUP_NOT_EXISTS);

      dl.setMaxElements(cmd.getMaxElements());

    } catch (DataSourceException e) {
      log.error(e,e);
      throw new CommandExecutionException(e.getMessage(), GroupEditCommand.ERR_SYS_ERROR);
    }
  }

   public Collection<DistrList> execute(GroupListCmd cmd) throws CommandExecutionException {
     if (log.isDebugEnabled())
      log.debug(cmd);

     String owner = cmd.getOwner();
     if (owner != null && !checkMsisdn(owner))
       throw new CommandExecutionException("Invalid owner: " + owner, GroupEditCommand.ERR_INV_OWNER);

     try {
       return listsDS.getDistrLists(owner);
     } catch (DataSourceException e) {
       log.error(e,e);
       throw new CommandExecutionException(e.getMessage(), GroupEditCommand.ERR_SYS_ERROR);
     }
  }

  private DistrList getDistrList(String name, String owner) throws DataSourceException, CommandExecutionException {
    DistrList dl = listsDS.getDistrList(name, owner);
    if (dl == null)
      throw new CommandExecutionException("Group: name=" + name + "; owner=" + owner + " does not exists", GroupEditCommand.ERR_GROUP_NOT_EXISTS);
    return dl;
  }

  // MEMBERS -----------------------------------------------------------------------------------------------------------

  public void execute(GroupAddMemberCmd cmd) throws CommandExecutionException {
    if (log.isDebugEnabled())
      log.debug(cmd);

    String member = cmd.getMember();
    String owner = cmd.getOwner();
    String name = cmd.getGroupName();

    checkGroup(name);

    if (isEmpty(member) || !checkMsisdn(member))
      throw new CommandExecutionException("Invalid member msisdn: " + member, GroupAddMemberCmd.ERR_INV_MEMBER);

    try {
      DistrList dl = getDistrList(name, owner);

      if (dl.containsMember(member)) {
        if (log.isDebugEnabled())
          log.debug("Member " + member + " already exists in " + name + ", skip it.");
        return;
      }

      if (dl.membersSize() == dl.getMaxElements())
        throw new CommandExecutionException("Members number exceeded in list " + name, GroupEditCommand.ERR_MEMBER_COUNT_EXCEEDED);

      dl.addMember(member);
      if (operators.getOperatorByAddress(member) != null && !dl.containsSubmitter(member))
        dl.addSubmitter(member);

    } catch (DataSourceException e) {
      log.error(e,e);
      throw new CommandExecutionException(e.getMessage(), GroupEditCommand.ERR_SYS_ERROR);
    }
  }

  public void execute(GroupRemoveMemberCmd cmd) throws CommandExecutionException {
    if (log.isDebugEnabled())
      log.debug(cmd);

    String name = cmd.getGroupName();
    String member = cmd.getMember();
    String owner = cmd.getOwner();

    checkGroup(name);

    if (isEmpty(member) || !checkMsisdn(member))
      throw new CommandExecutionException("Invalid member msisdn: " + member, GroupEditCommand.ERR_INV_MEMBER);

    try {
      DistrList dl = getDistrList(name, owner);

      if (!dl.removeMember(member))
        throw new CommandExecutionException("Member " + member + " does not exists in group " + name, GroupEditCommand.ERR_MEMBER_NOT_EXISTS);

      if (operators.getOperatorByAddress(member) != null)
        dl.removeSubmitter(member);

    } catch (DataSourceException e) {
      log.error(e,e);
      throw new CommandExecutionException(e.getMessage(), GroupEditCommand.ERR_SYS_ERROR);
    }
  }

  // SUBMITTERS --------------------------------------------------------------------------------------------------------

  public void execute(GroupAddSubmitterCmd cmd) throws CommandExecutionException {
    if (log.isDebugEnabled())
      log.debug(cmd);

    String submitter = cmd.getSubmitter();
    String name = cmd.getGroupName();
    String owner = cmd.getOwner();

    checkGroup(name);

    if (isEmpty(submitter) || !checkMsisdn(submitter))
      throw new CommandExecutionException("Invalid submitter msisdn: " + submitter, GroupEditCommand.ERR_INV_SUBMITTER);

    if (operators.getOperatorByAddress(submitter)  == null)
      throw new CommandExecutionException("Submitter is not allowed by operator: " + submitter, GroupEditCommand.ERR_INV_SUBMITTER);

    try {
      DistrList dl = getDistrList(name, owner);

      if (dl.containsSubmitter(submitter)) {
        if (log.isDebugEnabled())
          log.debug("Submitter " + submitter + " already exists in " + name + ", skip it.");
        return;
      }

      if (dl.submittersSize() == dl.getMaxElements())
        throw new CommandExecutionException("Submitters number exceeded in list " + name, GroupEditCommand.ERR_SUBMITTERS_COUNT_EXCEEDED);

      dl.addSubmitter(submitter);

    } catch (DataSourceException e) {
      log.error(e,e);
      throw new CommandExecutionException(e.getMessage(), GroupEditCommand.ERR_SYS_ERROR);
    }
  }

  public void execute(GroupRemoveSubmitterCmd cmd) throws CommandExecutionException {
    if (log.isDebugEnabled())
      log.debug(cmd);

    String submitter = cmd.getSubmitter();
    String name = cmd.getGroupName();
    String owner = cmd.getOwner();

    checkGroup(name);
    if (isEmpty(submitter) || !checkMsisdn(submitter))
      throw new CommandExecutionException("Invalid submitter msisdn", GroupEditCommand.ERR_INV_SUBMITTER);

    try {
      DistrList dl = getDistrList(name, owner);

      if (!dl.removeSubmitter(submitter))
        throw new CommandExecutionException("Submitter " + submitter + " does not exists in group " + name, GroupEditCommand.ERR_SUBMITTER_NOT_EXISTS);

    } catch (DataSourceException e) {
      log.error(e,e);
      throw new CommandExecutionException(e.getMessage(), GroupEditCommand.ERR_SYS_ERROR);
    }
  }

  // GROUP INFO --------------------------------------------------------------------------------------------------------

  public GroupInfo execute(GroupInfoCmd cmd) throws CommandExecutionException {
    if (log.isDebugEnabled())
      log.debug(cmd);

    String name = cmd.getGroupName();
    String owner = cmd.getOwner();

    checkGroup(name);

    try {
      DistrList dl = getDistrList(name, owner);

      GroupInfo r = new GroupInfo(dl.members(), dl.submitters());
      r.setMaxElements(dl.getMaxElements());
      return r;
    } catch (DataSourceException e) {
      log.error(e,e);
      throw new CommandExecutionException(e.getMessage(), GroupEditCommand.ERR_SYS_ERROR);
    }
  }

  // PRINCIPALS --------------------------------------------------------------------------------------------------------

  public void execute(PrincipalAddCmd cmd) throws CommandExecutionException {
    if (log.isDebugEnabled())
      log.debug(cmd);

    String principal = cmd.getAddress();
    if (isEmpty(principal) || !checkMsisdn(principal))
      throw new CommandExecutionException("Invalid principal address: " + principal , PrincipalAddCmd.ERR_INVALID_PRINCIPAL_ADDRESS);

    try {
      if (principalsDS.containsPrincipal(principal))
        throw new CommandExecutionException("Principal " + principal + " already exists", PrincipalAddCmd.ERR_PRINCIPAL_ALREADY_EXISTS);

      Principal p = new Principal(principal);
      p.setMaxLists(cmd.getMaxLists());
      p.setMaxMembersPerList(cmd.getMaxElements());

      principalsDS.addPrincipal(p);
    } catch (DataSourceException e) {
      log.error(e,e);
      throw new CommandExecutionException(e.getMessage(), Command.ERR_SYS_ERROR);
    }
  }

  public void execute(PrincipalRemoveCmd cmd) throws CommandExecutionException {
    if (log.isDebugEnabled())
      log.debug(cmd);

    String principal = cmd.getAddress();
    if (isEmpty(principal) || !checkMsisdn(principal))
      throw new CommandExecutionException("Invalid principal address: " + principal , PrincipalAddCmd.ERR_INVALID_PRINCIPAL_ADDRESS);

    try {
      if (!principalsDS.removePrincipal(principal))
        throw new CommandExecutionException("Principal " + principal + " does not exists", PrincipalRemoveCmd.ERR_PRINCIPAL_NOT_EXISTS);

      listsDS.removeDistrLists(principal);
    } catch (DataSourceException e) {
      log.error(e,e);
      throw new CommandExecutionException(e.getMessage(), Command.ERR_SYS_ERROR);
    }
  }

  public Collection<Principal> execute(PrincipalListCmd cmd) throws CommandExecutionException {
    if (log.isDebugEnabled())
      log.debug(cmd);

    try {
      return principalsDS.getPrincipals();
    } catch (DataSourceException e) {
      log.error(e,e);
      throw new CommandExecutionException(e.getMessage(), Command.ERR_SYS_ERROR);
    }
  }

  public Principal execute(PrincipalGetCmd cmd) throws CommandExecutionException {
    if (log.isDebugEnabled())
      log.debug(cmd);

    try {
      return principalsDS.getPrincipal(cmd.getAddress());
    } catch (DataSourceException e) {
      log.error(e,e);
      throw new CommandExecutionException(e.getMessage(), Command.ERR_SYS_ERROR);
    }
  }

  public void execute(PrincipalAlterCmd cmd) throws CommandExecutionException {
    if (log.isDebugEnabled())
      log.debug(cmd);

    String principal = cmd.getAddress();
    if (isEmpty(principal) || !checkMsisdn(principal))
      throw new CommandExecutionException("Invalid principal address: " + principal , PrincipalAddCmd.ERR_INVALID_PRINCIPAL_ADDRESS);

    try {
      Principal p = principalsDS.getPrincipal(principal);
      if (p != null) {
        principalsDS.removePrincipal(principal);
        Principal p1 = new Principal(principal);
        p1.setMaxLists((cmd.isAltLists()) ? cmd.getMaxLists() : p.getMaxLists());
        p1.setMaxMembersPerList((cmd.isAltElements()) ? cmd.getMaxElements() : p.getMaxMembersPerList());
        principalsDS.addPrincipal(p1);
      }
    } catch (DataSourceException e) {
      log.error(e,e);
      throw new CommandExecutionException(e.getMessage(), Command.ERR_SYS_ERROR);
    }
  }
}
