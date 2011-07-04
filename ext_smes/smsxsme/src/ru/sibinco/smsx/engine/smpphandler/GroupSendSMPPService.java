package ru.sibinco.smsx.engine.smpphandler;

import com.eyeline.sme.handler.SMPPRequest;
import com.eyeline.sme.handler.SMPPServiceException;
import com.eyeline.utils.config.ConfigException;
import com.eyeline.utils.config.properties.PropertiesConfig;
import com.logica.smpp.Data;
import org.apache.log4j.Category;
import ru.aurorisoft.smpp.Message;
import ru.aurorisoft.smpp.SMPPException;
import ru.sibinco.smsx.engine.service.*;
import ru.sibinco.smsx.engine.service.group.DeliveryStatus;
import ru.sibinco.smsx.engine.service.group.commands.GroupDeliveryReportCmd;
import ru.sibinco.smsx.engine.service.group.commands.GroupReplyCmd;
import ru.sibinco.smsx.engine.service.group.commands.GroupSendCmd;
import ru.sibinco.smsx.engine.service.group.commands.GroupSendCommand;

/**
 * User: artem
 * Date: 16.07.2008
 */

public class GroupSendSMPPService extends AbstractSMPPService {

  private static final Category log = Category.getInstance(GroupSendSMPPService.class);

  private String sendReport;
  private String unknownGroup;
  private String unknownSubmitter;

  protected void init(java.util.Properties properties) throws SMPPServiceException {
    super.init(properties);
    try {
      PropertiesConfig cfg = new PropertiesConfig(properties);
      sendReport = cfg.getString("send.report");
      unknownGroup = cfg.getString("unknown.group");
      unknownSubmitter = cfg.getString("unknown.submitter");
    } catch (ConfigException e) {
      throw new SMPPServiceException(e);
    }

    if (sendReport == null)
      throw new SMPPServiceException("Property send.report is not set in GroupSendSMPPService");
  }

  public boolean serve(SMPPRequest smppRequest) {
    if (smppRequest.getName().equals("group_send")) 
      return groupSend(smppRequest);
    else if (smppRequest.getInObj().getMessage().isReceipt())
      return receipt(smppRequest);
    else if (smppRequest.getName().equals("group_send_reply"))
      return groupReply(smppRequest);
    return false;
  }

  private boolean groupReply(SMPPRequest request) {
    final Message m = request.getInObj().getMessage();
    GroupReplyCmd cmd = new GroupReplyCmd();

    String msgText = request.getParameter("message");
    if (msgText == null)
      return false;

    cmd.setMessage(msgText);
    cmd.setSubmitter(m.getSourceAddress());
    cmd.setDestAddrSubunit(m.getDestAddrSubunit());
    cmd.setImsi(m.getImsi());
    cmd.setMscAddress(m.getMscAddress());
    cmd.setStorable(false);
    cmd.setSourceId(Command.SOURCE_SMPP);
    cmd.addExecutionObserver(new CommandObserver() {
      public void update(AsyncCommand command) {
        if (command.getStatus() == AsyncCommand.STATUS_SUCCESS) {
          DeliveryStatus[] statuses = ((GroupSendCommand)command).getDeliveryStatuses();
          int total = ((GroupSendCommand)command).getGroupSize();
          int sent = 0;
          for (DeliveryStatus status : statuses)
            if (status.getStatus() > 0) sent++;
          reply(m, sendReport.replace("{actual}", String.valueOf(sent)).replace("{total}", String.valueOf(total)));
        }
      }
    });

    try {
      Services.getInstance().getGroupService().execute(cmd);

      try {
        request.getInObj().respond(Data.ESME_ROK);
      } catch (SMPPException e) {
        log.error(e);
      }

      return true;
    } catch (CommandExecutionException e) {
      switch (e.getErrCode()) {
        case GroupSendCmd.ERR_UNKNOWN_GROUP:
          reply(m, unknownGroup);
          break;
        case GroupSendCmd.ERR_UNKNOWN_SUBMITTER:
          reply(m, unknownSubmitter);
          break;
        default:
          log.error(e,e);
      }
      return false;
    }
  }

  private boolean groupSend(SMPPRequest smppRequest) {
    try {
      smppRequest.getInObj().respond(Data.ESME_ROK);
    } catch (SMPPException e) {
      log.error(e, e);
    }

    final Message m = smppRequest.getInObj().getMessage();
    GroupSendCmd cmd = new GroupSendCmd();

    String group = smppRequest.getParameter("group");
    String owner;
    int i = group.indexOf('/');
    if (i > 0) {
      owner = group.substring(0, i);
      group = group.substring(i+1);
    } else
      owner = m.getSourceAddress();

    cmd.setGroupName(group);
    cmd.setOwner(owner);
    cmd.setSubmitter(m.getSourceAddress());
    cmd.setDestAddrSubunit(m.getDestAddrSubunit());
    cmd.setImsi(m.getImsi());
    cmd.setMscAddress(m.getMscAddress());
    cmd.setMessage(smppRequest.getParameter("message"));
    cmd.setStorable(false);
    cmd.setSourceId(Command.SOURCE_SMPP);

    cmd.addExecutionObserver(new CommandObserver() {
      public void update(AsyncCommand command) {
        if (command.getStatus() == AsyncCommand.STATUS_SUCCESS) {
          DeliveryStatus[] statuses = ((GroupSendCommand)command).getDeliveryStatuses();
          int sent = 0;
          for (DeliveryStatus status : statuses)
            if (status.getStatus() > 0) sent++;
          reply(m, sendReport.replace("{actual}", String.valueOf(sent)).replace("{total}", String.valueOf(statuses.length)));
        }
      }
    });

    try {
      Services.getInstance().getGroupService().execute(cmd);
    } catch (CommandExecutionException e) {
      switch (e.getErrCode()) {
        case GroupSendCmd.ERR_UNKNOWN_GROUP:
          reply(m, unknownGroup);
          break;
        case GroupSendCmd.ERR_UNKNOWN_SUBMITTER:
          reply(m, unknownSubmitter);
          break;
        default:
          log.error(e,e);
      }
    }
    return true;
  }

  private boolean receipt(SMPPRequest smppRequest) {
    final Message m = smppRequest.getInObj().getMessage();
    GroupDeliveryReportCmd cmd = new GroupDeliveryReportCmd();
    cmd.setUmr(m.getUserMessageReference());
    cmd.setDelivered(m.getMessageState() == Message.MSG_STATE_DELIVERED);
    cmd.setMsgId(Long.parseLong(m.getReceiptedMessageId()));

    try {
      Services.getInstance().getGroupService().execute(cmd);
    } catch (CommandExecutionException e) {
      log.error(e,e);
    }
    return true;
  }
}
