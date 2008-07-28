package ru.sibinco.smsx.engine.smpphandler;

import org.apache.log4j.Category;
import com.eyeline.sme.handler.SMPPRequest;
import com.eyeline.sme.handler.SMPPServiceException;
import com.eyeline.sme.handler.services.AbstractService;
import com.eyeline.utils.config.properties.PropertiesConfig;
import com.eyeline.utils.config.ConfigException;
import com.logica.smpp.Data;
import ru.aurorisoft.smpp.SMPPException;
import ru.sibinco.smsx.engine.service.group.commands.GroupDeliveryReportCmd;
import ru.sibinco.smsx.engine.service.group.commands.GroupEditAlterProfileCmd;
import ru.sibinco.smsx.engine.service.Services;
import ru.sibinco.smsx.engine.service.CommandExecutionException;

import java.util.Properties;

/**
 * User: artem
 * Date: 28.07.2008
 */

public class GroupEditSMPPService extends AbstractSMPPService {

  private static final Category log = Category.getInstance(GroupEditSMPPService.class);

  private String editLock;
  private String editUnlock;
  private String editAlertOn;
  private String editAlertOff;
  
  public void init(Properties initParams) throws SMPPServiceException {
    super.init(initParams);

    try {
      PropertiesConfig c = new PropertiesConfig(initParams);
      editLock = c.getString("edit.lock");
      editUnlock = c.getString("edit.unlock");
      editAlertOn = c.getString("edit.alert.on");
      editAlertOff = c.getString("edit.alert.off");
    } catch (ConfigException e) {
      throw new SMPPServiceException("Initialization error: " + e.getMessage());
    }
  }

  public boolean serve(SMPPRequest smppRequest) {

    GroupEditAlterProfileCmd cmd = new GroupEditAlterProfileCmd();
    cmd.setAddress(smppRequest.getInObj().getMessage().getSourceAddress());

    String notification;
    if (smppRequest.getName().equals("group_edit_off")) {
      cmd.setLockGroupEdit(true);
      notification = editLock;
    } else if (smppRequest.getName().equals("group_edit_on")) {
      cmd.setLockGroupEdit(false);
      notification = editUnlock;
    } else if (smppRequest.getName().equals("group_edit_notification_off")) {
      cmd.setSendNotifications(false);
      notification = editAlertOff;
    } else if (smppRequest.getName().equals("group_edit_notification_on")) {
      cmd.setSendNotifications(true);
      notification = editAlertOn;
    } else {
      log.error("Unknown request: " + smppRequest.getName());
      return false;
    }

    try {
      Services.getInstance().getGroupService().execute(cmd);
      try {
        smppRequest.getInObj().respond(Data.ESME_ROK);
      } catch (SMPPException e) {
        log.error(e);
      }
      sendMessage(smppRequest.getInObj().getMessage().getDestinationAddress(), smppRequest.getInObj().getMessage().getSourceAddress(), notification);
    } catch (CommandExecutionException e) {
      log.error(e,e);
      return false;
    }

    return true;
  }
}