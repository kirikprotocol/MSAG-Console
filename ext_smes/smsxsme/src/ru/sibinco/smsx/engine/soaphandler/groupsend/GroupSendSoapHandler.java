package ru.sibinco.smsx.engine.soaphandler.groupsend;

import org.apache.log4j.Category;
import ru.sibinco.smsx.engine.service.group.commands.GroupSendCmd;
import ru.sibinco.smsx.engine.service.group.commands.CheckStatusCmd;
import ru.sibinco.smsx.engine.service.*;
import ru.sibinco.smsx.engine.soaphandler.SOAPHandlerInitializationException;

class GroupSendSoapHandler implements GroupSend {

  private static final Category log = Category.getInstance(GroupSendSoapHandler.class);

  private static final int STATUS_ACCEPTED = 0;
  private static final int STATUS_DELIVERED = 1;
  private static final int STATUS_DELIVERY_ERROR = 2;
  private static final int STATUS_SYSTEM_ERROR = -1;
  private static final int STATUS_UNKNOWN_OWNER = -2;
  private static final int STATUS_UNKNOWN_GROUP = -3;
  private static final int STATUS_INVALID_MSGID = -4;

  GroupSendSoapHandler(String configDir) throws SOAPHandlerInitializationException {

  }

  public GroupSendResp sendSms(String groupName, String owner, String message, boolean express) throws java.rmi.RemoteException {
    if (log.isDebugEnabled())
      log.debug("SendSms: owner=" + owner + "; group=" + groupName + "; express=" + express);

    final GroupSendCmd cmd = new GroupSendCmd();
    cmd.setExpress(express)
      .setGroupName(groupName)
      .setOwner(owner)
      .setMessage(message)
      .setStorable(true)
      .setImsi("")
      .setMscAddress("")
      .setSourceId(AsyncCommand.SOURCE_SOAP);

    final GroupSendResp r = new GroupSendResp();
    try {
      long msgId = Services.getInstance().getGroupService().execute(cmd);
      r.setMsgId(String.valueOf(msgId));
      r.setStatus(STATUS_ACCEPTED);
    } catch (CommandExecutionException e) {
      r.setStatus(STATUS_SYSTEM_ERROR);
    }

    return r;
  }

  public GroupSendResp checkStatus(String msgId) throws java.rmi.RemoteException {
    if (log.isDebugEnabled())
      log.debug("CheckStatus: msgId=" + msgId);

    final CheckStatusCmd cmd = new CheckStatusCmd();
    final GroupSendResp resp = new GroupSendResp();
    resp.setMsgId(msgId);
    try {
      cmd.setMsgId(Long.valueOf(msgId));
    } catch (NumberFormatException e) {
      log.error("Invalid msgId=" + msgId);
      resp.setStatus(STATUS_INVALID_MSGID);
      return resp;
    }

    try {
      CheckStatusCmd.MessageStatus status = Services.getInstance().getGroupService().execute(cmd);
      resp.setStatus(getStatus(status));
    } catch (CommandExecutionException e) {
      log.error("Check status err.", e);
      resp.setStatus(STATUS_SYSTEM_ERROR);
    }

    return resp;
  }

  private static int getStatus(CheckStatusCmd.MessageStatus cmdStatus) {
    switch (cmdStatus) {
      case ACCEPTED: return STATUS_ACCEPTED;
      case DELIVERED: return STATUS_DELIVERED;
      case LIST_NOT_FOUND: return STATUS_UNKNOWN_GROUP;
      case OWNER_NOT_FOUND: return STATUS_UNKNOWN_OWNER;
      default: return STATUS_SYSTEM_ERROR;
    }
  }
}
