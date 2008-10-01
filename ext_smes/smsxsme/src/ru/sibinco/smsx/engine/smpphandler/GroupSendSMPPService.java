package ru.sibinco.smsx.engine.smpphandler;

import com.eyeline.sme.handler.SMPPRequest;
import com.logica.smpp.Data;
import ru.sibinco.smsx.engine.service.group.commands.GroupDeliveryReportCmd;
import ru.sibinco.smsx.engine.service.CommandExecutionException;
import ru.sibinco.smsx.engine.service.Services;
import ru.aurorisoft.smpp.SMPPException;
import org.apache.log4j.Category;

/**
 * User: artem
 * Date: 16.07.2008
 */

public class GroupSendSMPPService extends AbstractSMPPService {

  private static final Category log = Category.getInstance(GroupSendSMPPService.class);

  public boolean serve(SMPPRequest smppRequest) {
    try {
      smppRequest.getInObj().respond(Data.ESME_ROK);
    } catch (SMPPException e) {
      log.error(e);
    }

    GroupDeliveryReportCmd cmd = new GroupDeliveryReportCmd();
    cmd.setOwner(smppRequest.getInObj().getMessage().getDestinationAddress());
    cmd.setUmr(smppRequest.getInObj().getMessage().getUserMessageReference());

    if (smppRequest.getName().equalsIgnoreCase("group_send_delivery_ok")) {
      cmd.setDeliveryStatus(GroupDeliveryReportCmd.DeliveryStatus.DELIVERED);

    } else if (smppRequest.getName().equalsIgnoreCase("group_send_delivery_failed")) {
      String reason = smppRequest.getParameter("reason");
      if (reason != null) {
        if (reason.equalsIgnoreCase("dl.reason.list_not_found"))
          cmd.setDeliveryStatus(GroupDeliveryReportCmd.DeliveryStatus.LIST_NOT_FOUND);
        else
          cmd.setDeliveryStatus(GroupDeliveryReportCmd.DeliveryStatus.SYS_ERR);
      }     
    }

    try {
      Services.getInstance().getGroupService().execute(cmd);
    } catch (CommandExecutionException e) {
      log.error(e,e);
    }

    return true;
  }
}
