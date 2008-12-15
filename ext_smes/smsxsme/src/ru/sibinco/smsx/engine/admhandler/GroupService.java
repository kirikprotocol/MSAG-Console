package ru.sibinco.smsx.engine.admhandler;

import org.apache.log4j.Category;
import ru.sibinco.smsc.utils.admin.server.Command;
import ru.sibinco.smsc.utils.admin.server.Service;
import ru.sibinco.smsc.utils.admin.server.model.*;
import ru.sibinco.smsx.engine.service.CommandExecutionException;
import ru.sibinco.smsx.engine.service.Services;
import ru.sibinco.smsx.engine.service.group.commands.*;
import ru.sibinco.smsx.engine.service.group.datasource.DistrList;
import ru.sibinco.smsx.engine.service.group.datasource.Principal;
import ru.aurorisoft.smpp.Address;
import ru.aurorisoft.smpp.SMPPAddressException;

import java.util.*;

/**
 * User: artem
 * Date: 21.11.2008
 */
public class GroupService implements Service {

  private static final Category log = Category.getInstance(GroupService.class);

  public Response executeCommand(Command command) {
    String name = command.getMethodName();
    Response resp;
    try {
      if (name.equals("prc_list")) return listPrincipals(command);
      if (name.equals("prc_add")) return addPrincipal(command);
      if (name.equals("prc_delete")) return delPrincipal(command);
      if (name.equals("prc_get")) return getPrincipal(command);
      if (name.equals("prc_alter")) return alterPrincipal(command);
      if (name.equals("mem_add")) return addMember(command);
      if (name.equals("mem_delete")) return delMember(command);
      if (name.equals("mem_get")) return listMembers(command);
      if (name.equals("sbm_add")) return addSubmitter(command);
      if (name.equals("sbm_delete")) return delSubmitter(command);
      if (name.equals("sbm_list")) return listSubmitters(command);
      if (name.equals("dl_add")) return addList(command);
      if (name.equals("dl_delete")) return delList(command);
      if (name.equals("dl_get")) return getList(command);
      if (name.equals("dl_list")) return listLists(command);
      if (name.equals("dl_alter")) return alterList(command);
      if (name.equals("dl_rename")) return renameList(command);
      if (name.equals("dl_copy")) return copyList(command);

      resp = new Response(Type.STRING, "Unknown command");
      resp.setStatus(Response.Status.ERROR);

    } catch (CommandExecutionException e) {
      resp = new Response(Type.STRING, "");
      resp.setStatus(Response.Status.ERROR);
      resp.setError(getErrorText(e));
    } catch (Throwable e) {
      log.error(e,e);
      resp = new Response(Type.STRING, e.getMessage());
      resp.setStatus(Response.Status.ERROR);
      resp.setError(e.getMessage());
    }
    return resp;
  }


  private static String getErrorText(CommandExecutionException e) {
    log.warn(e.getMessage());
    switch (e.getErrCode()) {
      case GroupEditCommand.ERR_INV_GROUP_NAME            : return "InvalidListNameException";
      case GroupEditCommand.ERR_INV_OWNER                 : return "InvalidOwnerException";
      case GroupEditCommand.ERR_GROUP_ALREADY_EXISTS      : return "ListAlreadyExistsException";
      case GroupEditCommand.ERR_GROUPS_COUNT_EXCEEDED     : return "ListCountExceededException";
      case GroupEditCommand.ERR_GROUP_NOT_EXISTS          : return "ListNotExistsException";
      case GroupEditCommand.ERR_MEMBER_ALREADY_EXISTS     : return "MemberAlreadyExistsException";
      case GroupEditCommand.ERR_MEMBER_COUNT_EXCEEDED     : return "MemberCountExceededException";
      case GroupEditCommand.ERR_OWNER_NOT_EXISTS          : return "PrincipalNotExistsException";
      case GroupEditCommand.ERR_INV_MEMBER                : return "InvalidMemberException";
      case GroupEditCommand.ERR_MEMBER_NOT_EXISTS         : return "MemberNotExistsException";
      case GroupEditCommand.ERR_LOCKED_BY_OWNER           : return "LockedByOwnerException";
      case GroupEditCommand.ERR_SUBMITTER_NOT_EXISTS      : return "SubmitterNotExistsException";
      case GroupEditCommand.ERR_SUBMITTER_ALREADY_EXISTS  : return "SubmitterAlreadyExistsException";
      case GroupEditCommand.ERR_SUBMITTERS_COUNT_EXCEEDED : return "SubmitterCountExceededException";
      case GroupEditCommand.ERR_PRINCIPAL_ALREADY_EXISTS  : return "PrincipalAlreadyExistsException";
      case GroupEditCommand.ERR_INVALID_PRINCIPAL_ADDRESS : return "PrincipalInUseException";
      case GroupEditCommand.ERR_PRINCIPAL_NOT_EXISTS      : return "PrincipalNotExistsException";
      default: return "SystemErrorException";
    }

  }

  public Description createDescription() {
    Description d = new Description();
    Collection<Method> methods = new LinkedList<Method>();

    Method m = new Method("prc_list", Type.STRING_LIST);
    m.addParameter(new Parameter("address", Type.STRING));
    methods.add(m);

    m = new Method("prc_add", Type.STRING);
    m.addParameter(new Parameter("address", Type.STRING));
    m.addParameter(new Parameter("maxLists", Type.INT));
    m.addParameter(new Parameter("maxElements", Type.INT));
    methods.add(m);

    m = new Method("prc_delete", Type.STRING);
    m.addParameter(new Parameter("address", Type.STRING));
    methods.add(m);

    m = new Method("prc_get", Type.STRING_LIST);
    m.addParameter(new Parameter("address", Type.STRING));
    methods.add(m);

    m = new Method("prc_alter", Type.STRING);
    m.addParameter(new Parameter("address", Type.STRING));
    m.addParameter(new Parameter("maxLists", Type.INT));
    m.addParameter(new Parameter("maxElements", Type.INT));
    m.addParameter(new Parameter("altLists", Type.BOOL));
    m.addParameter(new Parameter("altElements", Type.BOOL));
    methods.add(m);

    m = new Method("mem_add", Type.STRING);
    m.addParameter(new Parameter("address", Type.STRING));
    m.addParameter(new Parameter("dlname", Type.STRING));
    methods.add(m);

    m = new Method("mem_delete", Type.STRING);
    m.addParameter(new Parameter("address", Type.STRING));
    m.addParameter(new Parameter("dlname", Type.STRING));
    methods.add(m);

    m = new Method("mem_get", Type.STRING_LIST);
    m.addParameter(new Parameter("dlname", Type.STRING));
    methods.add(m);

    m = new Method("sbm_add", Type.STRING);
    m.addParameter(new Parameter("address", Type.STRING));
    m.addParameter(new Parameter("dlname", Type.STRING));
    methods.add(m);

    m = new Method("sbm_delete", Type.STRING);
    m.addParameter(new Parameter("address", Type.STRING));
    m.addParameter(new Parameter("dlname", Type.STRING));
    methods.add(m);

    m = new Method("sbm_list", Type.STRING_LIST);
    m.addParameter(new Parameter("dlname", Type.STRING));
    methods.add(m);

    m = new Method("dl_add", Type.STRING);
    m.addParameter(new Parameter("owner", Type.STRING));
    m.addParameter(new Parameter("dlname", Type.STRING));
    m.addParameter(new Parameter("maxElements", Type.INT));
    methods.add(m);

    m = new Method("dl_delete", Type.STRING);
    m.addParameter(new Parameter("dlname", Type.STRING));
    methods.add(m);

    m = new Method("dl_get", Type.STRING_LIST);
    m.addParameter(new Parameter("dlname", Type.STRING));
    methods.add(m);

    m = new Method("dl_list", Type.STRING_LIST);
    m.addParameter(new Parameter("owners", Type.STRING_LIST));
    m.addParameter(new Parameter("names", Type.STRING_LIST));
    methods.add(m);

    m = new Method("dl_alter", Type.STRING);
    m.addParameter(new Parameter("dlname", Type.STRING));
    m.addParameter(new Parameter("maxElements", Type.INT));
    methods.add(m);

    m = new Method("dl_rename", Type.STRING);
    m.addParameter(new Parameter("newdlname", Type.STRING));
    m.addParameter(new Parameter("dlname", Type.STRING));
    methods.add(m);

    m = new Method("dl_copy", Type.STRING);
    m.addParameter(new Parameter("newdlname", Type.STRING));
    m.addParameter(new Parameter("dlname", Type.STRING));
    methods.add(m);

    d.addComponent(new Component("SMSC", methods));
    return d;
  }

  public String getName() {
    return "SMSC";
  }

  private static String getMsisdn(String address) {
    try {
      Address addr = new Address(address);
      return addr.getAddressString();
    } catch (SMPPAddressException e) {
      log.error(e,e);
      return address;
    }
  }

  private Response listPrincipals(Command cmd) throws CommandExecutionException {
    PrincipalListCmd c = new PrincipalListCmd();
    String addrPrefix = (String)cmd.getParameter("address").getValue();
    if (addrPrefix != null) {
      if (addrPrefix.length() == 0)
        addrPrefix = null;
      else
        addrPrefix = getMsisdn(addrPrefix);
    }
    Collection<Principal> res = Services.getInstance().getGroupService().execute(c);
    ArrayList<String> result = new ArrayList<String>(res.size());
    for (Principal p : res) {
      if (addrPrefix == null || p.getAddress().startsWith(addrPrefix)) {
        result.add(p.getAddress());
        result.add(String.valueOf(p.getMaxLists()));
        result.add(String.valueOf(p.getMaxMembersPerList()));
      }
    }
    return new Response(Type.STRING_LIST, result);
  }

  private Response addPrincipal(Command cmd) throws CommandExecutionException {
    PrincipalAddCmd c = new PrincipalAddCmd();
    c.setAddress(getMsisdn((String)cmd.getParameter("address").getValue()));
    c.setMaxLists((Integer)cmd.getParameter("maxLists").getValue());
    c.setMaxElements((Integer)cmd.getParameter("maxElements").getValue());
    Services.getInstance().getGroupService().execute(c);
    return new Response(Type.STRING, null);
  }

  private Response delPrincipal(Command cmd) throws CommandExecutionException {
    PrincipalRemoveCmd c = new PrincipalRemoveCmd();
    c.setAddress(getMsisdn((String)cmd.getParameter("address").getValue()));
    Services.getInstance().getGroupService().execute(c);
    return new Response(Type.STRING, null);
  }

  private Response getPrincipal(Command cmd) throws CommandExecutionException {
    PrincipalGetCmd c = new PrincipalGetCmd();
    c.setAddress(getMsisdn((String)cmd.getParameter("address").getValue()));
    Principal p = Services.getInstance().getGroupService().execute(c);
    if (p == null)
      return new Response(Type.STRING_LIST, Collections.emptyList());

    ArrayList<String> res = new ArrayList<String>(3);
    res.add(p.getAddress());
    res.add(String.valueOf(p.getMaxLists()));
    res.add(String.valueOf(p.getMaxMembersPerList()));
    return new Response(Type.STRING_LIST, res);
  }

  private Response alterPrincipal(Command cmd) throws CommandExecutionException {
    PrincipalAlterCmd c = new PrincipalAlterCmd();
    c.setAddress(getMsisdn((String)cmd.getParameter("address").getValue()));
    c.setMaxLists((Integer)cmd.getParameter("maxLists").getValue());
    c.setMaxElements((Integer)cmd.getParameter("maxElements").getValue());
    c.setAltLists((Boolean)cmd.getParameter("altLists").getValue());
    c.setAltElements((Boolean)cmd.getParameter("altElements").getValue());
    Services.getInstance().getGroupService().execute(c);
    return new Response(Type.STRING, null);
  }


  private static String getOwner(String dlName) {
    int i=dlName.indexOf('/');
    if (i == -1)
      return null;
    return '+' + dlName.substring(0, i);
  }

  private static String getGroupName(String dlName) {
    int i=dlName.indexOf('/');
    if (i == -1)
      return dlName;
    return dlName.substring(i + 1);
  }

  private Response addMember(Command cmd) throws CommandExecutionException {
    GroupAddMemberCmd c = new GroupAddMemberCmd();
    c.setMember(getMsisdn((String)cmd.getParameter("address").getValue()));
    c.setGroupName(getGroupName((String)cmd.getParameter("dlname").getValue()));
    c.setOwner(getOwner((String)cmd.getParameter("dlname").getValue()));
    Services.getInstance().getGroupService().execute(c);
    return new Response(Type.STRING, null);
  }

  private Response delMember(Command cmd) throws CommandExecutionException {
    GroupRemoveMemberCmd c = new GroupRemoveMemberCmd();
    c.setMember(getMsisdn((String)cmd.getParameter("address").getValue()));
    c.setGroupName(getGroupName((String)cmd.getParameter("dlname").getValue()));
    c.setOwner(getOwner((String)cmd.getParameter("dlname").getValue()));
    Services.getInstance().getGroupService().execute(c);
    return new Response(Type.STRING, null);
  }

  private Response listMembers(Command cmd) throws CommandExecutionException {
    GroupInfoCmd c = new GroupInfoCmd();
    c.setGroupName(getGroupName((String)cmd.getParameter("dlname").getValue()));
    c.setOwner(getOwner((String)cmd.getParameter("dlname").getValue()));
    GroupInfo res = Services.getInstance().getGroupService().execute(c);
    ArrayList<String> result = new ArrayList<String>(res.getMembers().size());
    for (String m : res.getMembers())
      result.add(m);
    return new Response(Type.STRING_LIST, result);
  }

  private Response addSubmitter(Command cmd) throws CommandExecutionException {
    GroupAddSubmitterCmd c = new GroupAddSubmitterCmd();
    c.setSubmitter(getMsisdn((String)cmd.getParameter("address").getValue()));
    c.setGroupName(getGroupName((String)cmd.getParameter("dlname").getValue()));
    c.setOwner(getOwner((String)cmd.getParameter("dlname").getValue()));
    Services.getInstance().getGroupService().execute(c);
    return new Response(Type.STRING, null);
  }

  private Response delSubmitter(Command cmd) throws CommandExecutionException {
    GroupRemoveSubmitterCmd c = new GroupRemoveSubmitterCmd();
    c.setSubmitter(getMsisdn((String)cmd.getParameter("address").getValue()));
    c.setGroupName(getGroupName((String)cmd.getParameter("dlname").getValue()));
    c.setOwner(getOwner((String)cmd.getParameter("dlname").getValue()));
    Services.getInstance().getGroupService().execute(c);
    return new Response(Type.STRING, null);
  }

  private Response listSubmitters(Command cmd) throws CommandExecutionException {
    GroupInfoCmd c = new GroupInfoCmd();
    c.setGroupName(getGroupName((String)cmd.getParameter("dlname").getValue()));
    c.setOwner(getOwner((String)cmd.getParameter("dlname").getValue()));
    GroupInfo res = Services.getInstance().getGroupService().execute(c);
    ArrayList<String> result = new ArrayList<String>(res.getSubmitters().size());
    for (String m : res.getSubmitters()) {
      result.add(m);
      result.add("0");
      result.add("0");
    }
    return new Response(Type.STRING_LIST, result);
  }

  private Response addList(Command cmd) throws CommandExecutionException {
    GroupAddCmd c = new GroupAddCmd();
    c.setGroupName(getGroupName((String)cmd.getParameter("dlname").getValue()));
    c.setOwner((String)cmd.getParameter("owner").getValue());
    c.setMaxElements((Integer)cmd.getParameter("maxElements").getValue());
    Services.getInstance().getGroupService().execute(c);
    return new Response(Type.STRING, null);
  }

  private Response delList(Command cmd) throws CommandExecutionException {
    GroupRemoveCmd c = new GroupRemoveCmd();
    c.setGroupName(getGroupName((String)cmd.getParameter("dlname").getValue()));
    c.setOwner(getOwner((String)cmd.getParameter("dlname").getValue()));
    Services.getInstance().getGroupService().execute(c);
    return new Response(Type.STRING, null);
  }

  private Response getList(Command cmd) throws CommandExecutionException {
    GroupInfoCmd c = new GroupInfoCmd();
    c.setGroupName(getGroupName((String)cmd.getParameter("dlname").getValue()));
    c.setOwner(getOwner((String)cmd.getParameter("dlname").getValue()));
    GroupInfo res = Services.getInstance().getGroupService().execute(c);
    ArrayList<String> result = new ArrayList<String>(3);
    result.add((String)cmd.getParameter("dlname").getValue());
    result.add(getOwner((String)cmd.getParameter("dlname").getValue()));
    result.add(String.valueOf(res.getMaxElements()));
    return new Response(Type.STRING_LIST, result);
  }

  private Response listLists(Command cmd) throws CommandExecutionException {

    List<String> owners = (List<String>)cmd.getParameter("owners").getValue();
    List<String> names = (List<String>)cmd.getParameter("names").getValue();

    LinkedList<String> result = new LinkedList<String>();
    LinkedList<DistrList> lists = new LinkedList<DistrList>();

    if (owners != null) {
      for (String o : owners) {
        GroupListCmd c = new GroupListCmd();
        c.setOwner(getMsisdn(o));
        lists.addAll(Services.getInstance().getGroupService().execute(c));
      }
    } else {
      GroupListCmd c = new GroupListCmd();
      lists.addAll(Services.getInstance().getGroupService().execute(c));
    }

    if (names != null) {
      for (Iterator<DistrList> iter = lists.iterator(); iter.hasNext();) {
        DistrList l = iter.next();

        boolean found = false;
        for (String name : names) {
          if (name.equals(l.getName())) {
            found = true;
            break;
          }
        }
        if (!found)
          iter.remove();
      }
    }
    
    for (DistrList l : lists) {
      String owner = l.getOwner();
      if (owner != null) {
        if (owner.charAt(0) == '+')
          owner = owner.substring(1);
        result.add(owner + '/' + l.getName());
      } else
        result.add(l.getName());
      result.add(l.getOwner());
      result.add(String.valueOf(l.getMaxElements()));
    }

    return new Response(Type.STRING_LIST, result);
  }

  private Response alterList(Command cmd) throws CommandExecutionException {
    GroupAlterCmd c = new GroupAlterCmd();
    c.setGroupName(getGroupName((String)cmd.getParameter("dlname").getValue()));
    c.setOwner(getOwner((String)cmd.getParameter("dlname").getValue()));
    c.setMaxElements((Integer)cmd.getParameter("maxElements").getValue());
    Services.getInstance().getGroupService().execute(c);
    return new Response(Type.STRING, null);
  }

  private Response renameList(Command cmd) throws CommandExecutionException {
    GroupRenameCmd c = new GroupRenameCmd();
    c.setGroupName(getGroupName((String)cmd.getParameter("dlname").getValue()));
    c.setOwner(getOwner((String)cmd.getParameter("dlname").getValue()));
    c.setNewGroupName(getGroupName((String)cmd.getParameter("newdlname").getValue()));
    Services.getInstance().getGroupService().execute(c);
    return new Response(Type.STRING, null);
  }

  private Response copyList(Command cmd) throws CommandExecutionException {
    GroupCopyCmd c = new GroupCopyCmd();
    c.setGroupName(getGroupName((String)cmd.getParameter("dlname").getValue()));
    c.setOwner(getOwner((String)cmd.getParameter("dlname").getValue()));
    c.setNewGroupName(getGroupName((String)cmd.getParameter("newdlname").getValue()));
    Services.getInstance().getGroupService().execute(c);
    return new Response(Type.STRING, null);
  }

}
