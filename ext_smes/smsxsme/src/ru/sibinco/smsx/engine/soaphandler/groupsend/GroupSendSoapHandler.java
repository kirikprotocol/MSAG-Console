package ru.sibinco.smsx.engine.soaphandler.groupsend;

import org.apache.log4j.Category;
import ru.sibinco.smsx.engine.service.group.commands.GroupSendCmd;
import ru.sibinco.smsx.engine.service.group.commands.GroupSendStatusCmd;
import ru.sibinco.smsx.engine.service.group.commands.GroupSendCommand;
import ru.sibinco.smsx.engine.service.group.DeliveryStatus;
import ru.sibinco.smsx.engine.service.*;
import ru.sibinco.smsx.engine.soaphandler.SOAPHandlerInitializationException;
import ru.aurorisoft.smpp.Message;

import java.io.File;

import com.eyeline.utils.config.properties.PropertiesConfig;
import com.eyeline.utils.config.ConfigException;
import com.eyeline.sme.smpp.OutgoingQueue;
import com.eyeline.sme.smpp.OutgoingObject;
import com.eyeline.sme.smpp.ShutdownedException;

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
  private final String sendReport;
  private final String serviceAddress;
  private final OutgoingQueue outQueue;

  GroupSendSoapHandler(String configDir, OutgoingQueue outQueue) throws SOAPHandlerInitializationException {
    final File configFile = new File(configDir, "soaphandlers/groupsendhandler.properties");

    try {
      final PropertiesConfig config = new PropertiesConfig();
      config.load(configFile);
      mscAddress = config.getString("msc.address");
      sendReport = config.getString("send.report");
      serviceAddress = config.getString("service.address");

    } catch (ConfigException e) {
      throw new SOAPHandlerInitializationException(e);
    }

    this.outQueue = outQueue;
  }

   private void sendMessage(String da, String text) {
    Message m = new Message();
    m.setSourceAddress(serviceAddress);
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

  public GroupSendResp sendSms(String groupName, final String owner, String message, boolean express) throws java.rmi.RemoteException {
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
    cmd.addExecutionObserver(new CommandObserver() {
      public void update(AsyncCommand command) {
        if (command.getStatus() == AsyncCommand.STATUS_SUCCESS) {
          DeliveryStatus s = ((GroupSendCommand)command).getDeliveryStatus();
          byte[] statuses = s.statuses();
          int sent = 0;
          for (byte status : statuses)
            if (status > 0) sent++;
          sendMessage(owner, sendReport.replace("{actual}", String.valueOf(sent)).replace("{total}", String.valueOf(statuses.length)));
        }
      }
    });

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
