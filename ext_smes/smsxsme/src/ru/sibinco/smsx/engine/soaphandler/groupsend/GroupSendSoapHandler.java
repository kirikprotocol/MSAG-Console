package ru.sibinco.smsx.engine.soaphandler.groupsend;

import org.apache.log4j.Category;
import ru.sibinco.smsx.engine.service.group.commands.GroupSendCmd;
import ru.sibinco.smsx.engine.service.group.commands.GroupSendStatusCmd;
import ru.sibinco.smsx.engine.service.group.commands.GroupSendCommand;
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

  private static final int SEND_STATUS_ACCEPTED = 0;
  private static final int SEND_STATUS_SYS_ERR = -1;
  private static final int SEND_STATUS_UNKNOWN_OWNER = -2;
  private static final int SEND_STATUS_UNKNOWN_GROUP = -3;
  private static final int SEND_STATUS_INV_MSGID = -4;

  private static final int DELIVERY_STATUS_ACCEPTED = 0;
  private static final int DELIVERY_STATUS_DELIVERED = 1;
  private static final int DELIVERY_STATUS_SYS_ERR = -1;
  private static final int DELIVERY_STATUS_NOT_ENOUGHT_MONEY = -2;

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
    long start = System.currentTimeMillis();
    final GroupSendResp r = new GroupSendResp();
    try {
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
            ru.sibinco.smsx.engine.service.group.DeliveryStatus[] statuses = ((GroupSendCommand) command).getDeliveryStatuses();
            int sent = 0;
            for (ru.sibinco.smsx.engine.service.group.DeliveryStatus status : statuses)
              if (status.getStatus() > 0) sent++;
            sendMessage(owner, sendReport.replace("{actual}", String.valueOf(sent)).replace("{total}", String.valueOf(statuses.length)));
          }
        }
      });

      try {
        long msgId = Services.getInstance().getGroupService().execute(cmd);
        r.setMsgId(String.valueOf(msgId));
        r.setStatus(SEND_STATUS_ACCEPTED);
      } catch (CommandExecutionException e) {
        log.error("Send sms err : " + e.getMessage());
        switch (e.getErrCode()) {
          case GroupSendCmd.ERR_UNKNOWN_GROUP:
            r.setStatus(SEND_STATUS_UNKNOWN_GROUP);
            break;
          case GroupSendCmd.ERR_UNKNOWN_OWNER:
            r.setStatus(SEND_STATUS_UNKNOWN_OWNER);
            break;
          default:
            r.setStatus(SEND_STATUS_SYS_ERR);
            log.error(e, e);
        }
      }

    } catch (Throwable e) {
      log.error(e, e);
      r.setStatus(SEND_STATUS_SYS_ERR);
    } finally {
      if (log.isDebugEnabled())
        log.debug("Time=" + (System.currentTimeMillis() - start));
    }

    return r;
  }

  public GroupSendResp checkStatus(String msgId) throws java.rmi.RemoteException {
    long start = System.currentTimeMillis();
    final GroupSendResp resp = new GroupSendResp();
    try {
      if (log.isDebugEnabled())
        log.debug("CheckStatus: msgId=" + msgId);

      final GroupSendStatusCmd cmd = new GroupSendStatusCmd();
      resp.setMsgId(msgId);
      try {
        cmd.setMsgId(Integer.parseInt(msgId));
      } catch (NumberFormatException e) {
        log.error("Invalid msgId=" + msgId);
        resp.setStatus(SEND_STATUS_INV_MSGID);
        return resp;
      }

      try {
        ru.sibinco.smsx.engine.service.group.DeliveryStatus[] statuses = Services.getInstance().getGroupService().execute(cmd);
        if (statuses != null) {
          DeliveryStatus[] result = new DeliveryStatus[statuses.length];
          for (int i = 0; i < statuses.length; i++) {
            int status;
            switch (statuses[i].getStatus()) {
              case ru.sibinco.smsx.engine.service.group.DeliveryStatus.ACCEPTED:
              case ru.sibinco.smsx.engine.service.group.DeliveryStatus.SENT:
                status = DELIVERY_STATUS_ACCEPTED;
                break;
              case ru.sibinco.smsx.engine.service.group.DeliveryStatus.DELIVERED:
                status = DELIVERY_STATUS_DELIVERED;
                break;
              case ru.sibinco.smsx.engine.service.group.DeliveryStatus.NOT_DELIVERED:
                status = DELIVERY_STATUS_NOT_ENOUGHT_MONEY;
                break;
              default:
                status = DELIVERY_STATUS_SYS_ERR;
            }
            result[i] = new DeliveryStatus(statuses[i].getAddress(), status);
          }

          resp.setDeliveryStatuses(result);
          resp.setStatus(SEND_STATUS_ACCEPTED);
        } else
          resp.setStatus(SEND_STATUS_INV_MSGID);

      } catch (CommandExecutionException e) {
        log.error("Check status err.", e);
        resp.setStatus(SEND_STATUS_SYS_ERR);
      }

    } catch (Throwable e) {
      log.error("Check status err.", e);
      resp.setStatus(SEND_STATUS_SYS_ERR);
    } finally {
      if (log.isDebugEnabled())
        log.debug("Time=" + (System.currentTimeMillis() - start));
    }

    return resp;
  }

}
