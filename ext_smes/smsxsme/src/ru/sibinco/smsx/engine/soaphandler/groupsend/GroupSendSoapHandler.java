package ru.sibinco.smsx.engine.soaphandler.groupsend;

import org.apache.log4j.Category;
import ru.sibinco.smsx.engine.service.group.commands.GroupSendCmd;
import ru.sibinco.smsx.engine.service.group.commands.GroupSendStatusCmd;
import ru.sibinco.smsx.engine.service.group.DeliveryStatus;
import ru.sibinco.smsx.engine.service.*;
import ru.sibinco.smsx.engine.soaphandler.SOAPHandlerInitializationException;

import java.io.File;

import com.eyeline.utils.config.properties.PropertiesConfig;
import com.eyeline.utils.config.ConfigException;

class GroupSendSoapHandler implements GroupSend {

  private static final Category log = Category.getInstance(GroupSendSoapHandler.class);

  private static final int STATUS_ACCEPTED = 0;
  private static final int STATUS_DELIVERED = 1;
  private static final int STATUS_SYSTEM_ERROR = -1;
  private static final int STATUS_UNKNOWN_OWNER = -2;
  private static final int STATUS_UNKNOWN_GROUP = -3;
  private static final int STATUS_INVALID_MSGID = -4;
  private static final int STATUS_UNKNOWN = -5;

  private final String mscAddress;

  GroupSendSoapHandler(String configDir) throws SOAPHandlerInitializationException {
    final File configFile = new File(configDir, "soaphandlers/groupsendhandler.properties");

    try {
      final PropertiesConfig config = new PropertiesConfig();
      config.load(configFile);
      mscAddress = config.getString("msc.address");
    } catch (ConfigException e) {
      throw new SOAPHandlerInitializationException(e);
    }
  }

  public GroupSendResp sendSms(String groupName, String owner, String message, boolean express) throws java.rmi.RemoteException {
    if (log.isDebugEnabled())
      log.debug("SendSms: owner=" + owner + "; group=" + groupName + "; express=" + express);

    final GroupSendCmd cmd = new GroupSendCmd();
    cmd.setDestAddrSubunit(express ? 1 : 0);
    cmd.setGroupName(groupName);
    cmd.setOwner(owner);
    cmd.setSubmitter(owner);
    cmd.setMessage(message);
    cmd.setStorable(true);
    cmd.setMscAddress(mscAddress);
    cmd.setSourceId(AsyncCommand.SOURCE_SOAP);

    final GroupSendResp r = new GroupSendResp();
    try {
      long msgId = Services.getInstance().getGroupService().execute(cmd);
      r.setMsgId(String.valueOf(msgId));
      r.setStatus(STATUS_ACCEPTED);
    } catch (CommandExecutionException e) {
      log.error("Send sms err.", e);
      r.setStatus(STATUS_SYSTEM_ERROR);
    }

    return r;
  }

  public GroupSendResp checkStatus(String msgId) throws java.rmi.RemoteException {
    if (log.isDebugEnabled())
      log.debug("CheckStatus: msgId=" + msgId);

    final GroupSendStatusCmd cmd = new GroupSendStatusCmd();
    final GroupSendResp resp = new GroupSendResp();
    resp.setMsgId(msgId);
    try {
      cmd.setMsgId(Integer.parseInt(msgId));
    } catch (NumberFormatException e) {
      log.error("Invalid msgId=" + msgId);
      resp.setStatus(STATUS_INVALID_MSGID);
      return resp;
    }

    try {
      DeliveryStatus status = Services.getInstance().getGroupService().execute(cmd);
      resp.setStatus(getStatus(status));
    } catch (CommandExecutionException e) {
      log.error("Check status err.", e);
      resp.setStatus(STATUS_SYSTEM_ERROR);
    }

    return resp;
  }

  private static int getStatus(DeliveryStatus status) {
    return status == null ? STATUS_UNKNOWN : status.statuses()[0]/*STATUS_ACCEPTED*/;     
  }
}
