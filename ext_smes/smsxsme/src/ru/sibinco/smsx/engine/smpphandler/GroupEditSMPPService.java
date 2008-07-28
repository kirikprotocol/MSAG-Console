package ru.sibinco.smsx.engine.smpphandler;

import org.apache.log4j.Category;
import com.eyeline.sme.handler.SMPPRequest;
import com.logica.smpp.Data;
import ru.aurorisoft.smpp.SMPPException;
import ru.sibinco.smsx.engine.service.group.commands.GroupDeliveryReportCmd;
import ru.sibinco.smsx.engine.service.group.commands.GroupEditAlterProfileCmd;
import ru.sibinco.smsx.engine.service.Services;
import ru.sibinco.smsx.engine.service.CommandExecutionException;

/**
 * User: artem
 * Date: 28.07.2008
 */

public class GroupEditSMPPService extends AbstractSMPPService {

  private static final Category log = Category.getInstance(GroupEditSMPPService.class);

  public boolean serve(SMPPRequest smppRequest) {
    try {
      smppRequest.getInObj().respond(Data.ESME_ROK);
    } catch (SMPPException e) {
      log.error(e);
    }

    GroupEditAlterProfileCmd cmd = new GroupEditAlterProfileCmd();
    cmd.setAddress(smppRequest.getInObj().getMessage().getSourceAddress());

    if (smppRequest.getName().equals("group_edit_off")) {
      cmd.setLockGroupEdit(true);
    } else if (smppRequest.getName().equals("group_edit_on")) {
      cmd.setLockGroupEdit(false);
    } else if (smppRequest.getName().equals("group_edit_notification_off")) {
      cmd.setSendNotifications(false);
    } else if (smppRequest.getName().equals("group_edit_notification_on")) {
      cmd.setSendNotifications(true);
    }

    try {
      Services.getInstance().getGroupService().execute(cmd);
    } catch (CommandExecutionException e) {
      log.error(e,e);
    }

    return true;
  }
}