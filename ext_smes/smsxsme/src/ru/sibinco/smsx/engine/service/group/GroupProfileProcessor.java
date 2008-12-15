package ru.sibinco.smsx.engine.service.group;

import ru.sibinco.smsx.engine.service.group.commands.GroupEditAlterProfileCmd;
import ru.sibinco.smsx.engine.service.group.commands.GroupEditGetProfileCmd;
import ru.sibinco.smsx.engine.service.group.commands.GroupEditCommand;
import ru.sibinco.smsx.engine.service.group.datasource.GroupEditProfile;
import ru.sibinco.smsx.engine.service.group.datasource.GroupEditProfileDataSource;
import ru.sibinco.smsx.engine.service.CommandExecutionException;
import ru.sibinco.smsx.engine.service.Command;
import ru.sibinco.smsx.utils.DataSourceException;

/**
 * User: artem
 * Date: 03.12.2008
 */
class GroupProfileProcessor implements GroupEditAlterProfileCmd.Receiver,
                                              GroupEditGetProfileCmd.Receiver {

  private final GroupEditProfileDataSource profileDS;

  public GroupProfileProcessor(GroupEditProfileDataSource profileDS) {
    this.profileDS = profileDS;
  }

  public void execute(GroupEditAlterProfileCmd cmd) throws CommandExecutionException {
    final GroupEditProfile p = new GroupEditProfile(cmd.getAddress());
    p.setLockGroupEdit(cmd.getLockGroupEdit());
    p.setSendSmsNotification(cmd.getSendNotifications());

    try {
      profileDS.saveProfile(p);
    } catch (DataSourceException e) {
      throw new CommandExecutionException(e.getMessage(), Command.ERR_SYS_ERROR);
    }
  }

  public GroupEditGetProfileCmd.Result execute(GroupEditGetProfileCmd cmd) throws CommandExecutionException {
    try {
      GroupEditProfile profile = profileDS.loadProfile(cmd.getAddress());
      GroupEditGetProfileCmd.Result result = new GroupEditGetProfileCmd.Result();
      if (profile != null) {
        result.lockEdit = profile.isLockGroupEdit();
        result.sendNotification = profile.isSendSmsNotification();
      } else {
        result.lockEdit = false;
        result.sendNotification = true;
      }
      return result;
    } catch (DataSourceException e) {
      throw new CommandExecutionException(e.getMessage(), GroupEditCommand.ERR_SYS_ERROR);
    }
  }

}
