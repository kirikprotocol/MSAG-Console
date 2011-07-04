package ru.sibinco.smsx.engine.smpphandler;

import com.eyeline.sme.handler.SMPPRequest;
import com.eyeline.sme.handler.SMPPServiceException;
import com.eyeline.utils.config.ConfigException;
import com.eyeline.utils.config.properties.PropertiesConfig;
import com.logica.smpp.Data;
import org.apache.log4j.Category;
import ru.aurorisoft.smpp.SMPPException;
import ru.sibinco.smsx.engine.service.CommandExecutionException;
import ru.sibinco.smsx.engine.service.Services;
import ru.sibinco.smsx.engine.service.group.commands.*;
import ru.sibinco.smsx.engine.service.group.datasource.DistrList;

import java.util.Collection;
import java.util.Properties;

/**
 * User: artem
 * Date: 28.07.2008
 */

public class GroupEditSMPPService extends AbstractSMPPService {

  private static final Category log = Category.getInstance(GroupEditSMPPService.class);

  private String listempty;
  private String listerr;
  private String addok;
  private String adderr;
  private String delok;
  private String delerr;
  private String maddok;
  private String madderr;
  private String saddok;
  private String sadderr;
  private String mdelok;
  private String mdelerr;
  private String sdelok;
  private String sdelerr;
  private String mlistempty;
  private String mlisterr;
  private String slisterr;
  private String addsallok;
  private String addsallerr;
  private String delsallerr;
  private String delsallok;

  private String err_group_already_exists;
  private String err_group_not_exists;
  private String err_groups_count_exceeded;
  private String err_inv_group_name;
  private String err_inv_member;
  private String err_inv_owner;
  private String err_locked_by_owner;
  private String err_member_already_exists;
  private String err_members_count_exceeded;
  private String err_member_not_exists;
  private String err_owner_not_exists;
  private String err_submitter_already_exists;
  private String err_submitter_not_exists;
  private String err_submitters_count_exceeded;
  private String err_inv_submitter;
  private String err_system;


  public void init(Properties initParams) throws SMPPServiceException {
    super.init(initParams);

    try {
      PropertiesConfig c = new PropertiesConfig(initParams);
      listempty = c.getString("listempty");
      listerr = c.getString("listerr");
      addok = c.getString("addok");
      adderr = c.getString("adderr");
      delok = c.getString("delok");
      delerr = c.getString("delerr");
      maddok = c.getString("maddok");
      madderr = c.getString("madderr");
      saddok = c.getString("saddok");
      sadderr = c.getString("sadderr");
      mdelok = c.getString("mdelok");
      mdelerr = c.getString("mdelerr");
      sdelok = c.getString("sdelok");
      sdelerr = c.getString("sdelerr");
      mlistempty = c.getString("mlistempty");
      mlisterr = c.getString("mlisterr");
      slisterr = c.getString("slisterr");
      addsallok = c.getString("addsallok");
      addsallerr = c.getString("addsallerr");
      delsallerr = c.getString("delsallerr");
      delsallok = c.getString("delsallok");

      err_group_already_exists = c.getString("err_group_already_exists");
      err_group_not_exists = c.getString("err_group_not_exists");
      err_groups_count_exceeded = c.getString("err_groups_count_exceeded");
      err_inv_group_name = c.getString("err_inv_group_name");
      err_inv_member = c.getString("err_inv_member");
      err_inv_owner = c.getString("err_inv_owner");
      err_locked_by_owner = c.getString("err_locked_by_owner");
      err_member_already_exists = c.getString("err_member_already_exists");
      err_members_count_exceeded = c.getString("err_members_count_exceeded");
      err_member_not_exists = c.getString("err_member_not_exists");
      err_owner_not_exists = c.getString("err_owner_not_exists");
      err_submitter_already_exists = c.getString("err_submitter_already_exists");
      err_submitter_not_exists = c.getString("err_submitter_not_exists");
      err_submitters_count_exceeded = c.getString("err_submitters_count_exceeded");
      err_inv_submitter = c.getString("err_inv_submitter");
      err_system = c.getString("err_system");

    } catch (ConfigException e) {
      throw new SMPPServiceException("Initialization error: " + e.getMessage());
    }
  }

  public boolean serve(SMPPRequest req) {
    try {
      req.getInObj().respond(Data.ESME_ROK);
    } catch (SMPPException e) {
      log.error(e);
    }

    if (req.getName().equals("group_edit_list")) return list(req);
    else if (req.getName().equals("group_edit_add")) return add(req);
    else if (req.getName().equals("group_edit_del")) return del(req);
    else if (req.getName().equals("group_edit_addm")) return addm(req);
    else if (req.getName().equals("group_edit_delm")) return delm(req);
    else if (req.getName().equals("group_edit_adds")) return adds(req);
    else if (req.getName().equals("group_edit_dels")) return dels(req);
    else if (req.getName().equals("group_edit_list_members")) return members(req);
    else if (req.getName().equals("group_edit_list_submitters")) return submitters(req);
    else if (req.getName().equals("group_edit_adds_all")) return addsAll(req);
    else if (req.getName().equals("group_edit_dels_all")) return delsAll(req);
    else {
      log.error("Unknown request: " + req.getName());
      return false;
    }
  }

  private void reply(SMPPRequest req, String message) {
    reply(req.getInObj().getMessage(), message);
  }

  private String getReason(CommandExecutionException e) {
    switch (e.getErrCode()) {
      case GroupEditCommand.ERR_GROUP_ALREADY_EXISTS: return err_group_already_exists;
      case GroupEditCommand.ERR_GROUP_NOT_EXISTS: return err_group_not_exists;
      case GroupEditCommand.ERR_GROUPS_COUNT_EXCEEDED: return err_groups_count_exceeded;
      case GroupEditCommand.ERR_INV_GROUP_NAME: return err_inv_group_name;
      case GroupEditCommand.ERR_INV_MEMBER: return err_inv_member;
      case GroupEditCommand.ERR_INV_OWNER: return err_inv_owner;
      case GroupEditCommand.ERR_LOCKED_BY_OWNER: return err_locked_by_owner;
      case GroupEditCommand.ERR_MEMBER_ALREADY_EXISTS: return err_member_already_exists;
      case GroupEditCommand.ERR_MEMBER_COUNT_EXCEEDED: return err_members_count_exceeded;
      case GroupEditCommand.ERR_MEMBER_NOT_EXISTS: return err_member_not_exists;
      case GroupEditCommand.ERR_OWNER_NOT_EXISTS: return err_owner_not_exists;
      case GroupEditCommand.ERR_SUBMITTER_ALREADY_EXISTS: return err_submitter_already_exists;
      case GroupEditCommand.ERR_SUBMITTER_NOT_EXISTS: return err_submitter_not_exists;
      case GroupEditCommand.ERR_SUBMITTERS_COUNT_EXCEEDED: return err_submitters_count_exceeded;
      case GroupEditCommand.ERR_INV_SUBMITTER: return err_inv_submitter;
      default: return err_system;
    }
  }

  private static String prepareMsisdn(String msisdn) {
    if (msisdn != null && msisdn.length() > 0) {
      if (msisdn.charAt(0) == '8' && msisdn.length() == 11)
        msisdn = "+7" + msisdn.substring(1);
      if (msisdn.charAt(0) == '7')
        msisdn = '+' + msisdn;
    }
    return msisdn;
  }

  private boolean list(SMPPRequest req) {
    try {
      GroupListCmd c = new GroupListCmd();
      c.setOwner(req.getInObj().getMessage().getSourceAddress());

      Collection<DistrList> lists = Services.getInstance().getGroupService().execute(c);
      if (lists.isEmpty()) {
        reply(req, listempty);
        return true;
      }

      StringBuilder sb = new StringBuilder();
      for (DistrList list : lists) {
        if (sb.length() != 0)
          sb.append(',');
        sb.append(list.getName());
      }

      reply(req, sb.toString());
      return true;
    } catch (CommandExecutionException e) {
      reply(req, listerr.replace("{reason}", getReason(e)));      
      return true;
    }
  }

  private boolean add(SMPPRequest req) {
    try {
      GroupAddCmd c = new GroupAddCmd();
      c.setGroupName(req.getParameter("group"));
      c.setOwner(req.getInObj().getMessage().getSourceAddress());
      Services.getInstance().getGroupService().execute(c);

      reply(req, addok);
      return true;
    } catch (CommandExecutionException e) {
      reply(req, adderr.replace("{reason}", getReason(e)));
      return true;
    }
  }

  private boolean del(SMPPRequest req) {
    try {
      GroupRemoveCmd c = new GroupRemoveCmd();
      c.setGroupName(req.getParameter("group"));
      c.setOwner(req.getInObj().getMessage().getSourceAddress());

      Services.getInstance().getGroupService().execute(c);

      reply(req, delok);
      return true;
    } catch (CommandExecutionException e) {
      reply(req, delerr.replace("{reason}", getReason(e)));
      return true;
    }
  }

  private boolean addm(SMPPRequest req) {
    try {
      GroupAddMemberCmd c = new GroupAddMemberCmd();
      c.setGroupName(req.getParameter("group"));
      c.setMember(prepareMsisdn(req.getParameter("member")));
      c.setOwner(req.getInObj().getMessage().getSourceAddress());

      Services.getInstance().getGroupService().execute(c);

      reply(req, maddok);
      return true;
    } catch (CommandExecutionException e) {
      reply(req, madderr.replace("{reason}", getReason(e)));
      return true;
    }
  }

  private boolean delm(SMPPRequest req) {
    try {
      GroupRemoveMemberCmd c = new GroupRemoveMemberCmd();
      c.setGroupName(req.getParameter("group"));
      c.setMember(prepareMsisdn(req.getParameter("member")));
      c.setOwner(req.getInObj().getMessage().getSourceAddress());

      Services.getInstance().getGroupService().execute(c);

      reply(req, mdelok);
      return true;
    } catch (CommandExecutionException e) {
      reply(req, mdelerr.replace("{reason}", getReason(e)));
      return true;
    }
  }

  private boolean adds(SMPPRequest req) {
    try {
      GroupAddSubmitterCmd c = new GroupAddSubmitterCmd();
      c.setGroupName(req.getParameter("group"));
      c.setSubmitter(prepareMsisdn(req.getParameter("submitter")));
      c.setOwner(req.getInObj().getMessage().getSourceAddress());

      Services.getInstance().getGroupService().execute(c);

      reply(req, saddok);
      return true;
    } catch (CommandExecutionException e) {
      reply(req, sadderr.replace("{reason}", getReason(e)));
      return true;
    }
  }

  private boolean dels(SMPPRequest req) {
    try {
      GroupRemoveSubmitterCmd c = new GroupRemoveSubmitterCmd();
      c.setGroupName(req.getParameter("group"));
      c.setSubmitter(prepareMsisdn(req.getParameter("submitter")));
      c.setOwner(req.getInObj().getMessage().getSourceAddress());

      Services.getInstance().getGroupService().execute(c);

      reply(req, sdelok);
      return true;
    } catch (CommandExecutionException e) {
      reply(req, sdelerr.replace("{reason}", getReason(e)));
      return true;
    }
  }

  private boolean members(SMPPRequest req) {
    try {
      GroupInfoCmd c = new GroupInfoCmd();
      c.setGroupName(req.getParameter("group"));
      c.setOwner(req.getInObj().getMessage().getSourceAddress());

      GroupInfo info = Services.getInstance().getGroupService().execute(c);
      if (info.getMembers().isEmpty()) {
        reply(req, mlistempty);
        return true;
      }

      StringBuilder sb = new StringBuilder();
      for (String member : info.getMembers()) {
        if (sb.length() != 0)
          sb.append(',');
        sb.append(member);
      }

      reply(req, sb.toString());
      return true;
    } catch (CommandExecutionException e) {
      reply(req, mlisterr.replace("{reason}", getReason(e)));
      return true;
    }
  }

  private boolean submitters(SMPPRequest req) {
    try {
      GroupInfoCmd c = new GroupInfoCmd();
      c.setGroupName(req.getParameter("group"));
      c.setOwner(req.getInObj().getMessage().getSourceAddress());

      GroupInfo info = Services.getInstance().getGroupService().execute(c);
      if (info.getSubmitters().isEmpty()) {
        reply(req, mlistempty);
        return true;
      }

      StringBuilder sb = new StringBuilder();
      for (String submitter : info.getSubmitters()) {
        if (sb.length() != 0)
          sb.append(',');
        sb.append(submitter);
      }

      reply(req, sb.toString());
      return true;
    } catch (CommandExecutionException e) {
      reply(req, slisterr.replace("{reason}", getReason(e)));
      return true;
    }
  }

  private boolean addsAll(SMPPRequest req) {
    try {
      String owner = req.getInObj().getMessage().getSourceAddress();
      String name = req.getParameter("group");

      GroupInfoCmd c = new GroupInfoCmd();
      c.setGroupName(name);
      c.setOwner(owner);

      GroupInfo info = Services.getInstance().getGroupService().execute(c);
      if (info.getMembers().isEmpty()) {
        reply(req, mlistempty);
        return true;
      }

      for (String member : info.getMembers()) {
        if (!member.equals(owner) && !info.getSubmitters().contains(member)) {
          GroupAddSubmitterCmd cmd = new GroupAddSubmitterCmd();
          cmd.setGroupName(name);
          cmd.setOwner(owner);
          cmd.setSubmitter(member);
          Services.getInstance().getGroupService().execute(cmd);
        }
      }

      reply(req, addsallok);
      return true;
    } catch (CommandExecutionException e) {
      reply(req, addsallerr.replace("{reason}", getReason(e)));
      return true;
    }
  }

  private boolean delsAll(SMPPRequest req) {
    try {
      String owner = req.getInObj().getMessage().getSourceAddress();
      String name = req.getParameter("group");

      GroupInfoCmd c = new GroupInfoCmd();
      c.setGroupName(name);
      c.setOwner(owner);

      GroupInfo info = Services.getInstance().getGroupService().execute(c);
      if (info.getMembers().isEmpty()) {
        reply(req, mlistempty);
        return true;
      }

      for (String member : info.getMembers()) {
        if (!member.equals(owner) && info.getSubmitters().contains(member)) {
          GroupRemoveSubmitterCmd cmd = new GroupRemoveSubmitterCmd();
          cmd.setGroupName(name);
          cmd.setOwner(owner);
          cmd.setSubmitter(member);
          Services.getInstance().getGroupService().execute(cmd);
        }
      }

      reply(req, delsallok);
      return true;
    } catch (CommandExecutionException e) {
      reply(req, delsallerr.replace("{reason}", getReason(e)));
      return true;
    }
  }
}