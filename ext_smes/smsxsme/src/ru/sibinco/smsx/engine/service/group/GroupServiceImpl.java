package ru.sibinco.smsx.engine.service.group;

import ru.sibinco.smsx.engine.service.group.commands.*;
import ru.sibinco.smsx.engine.service.group.datasource.GroupSendDataSource;
import ru.sibinco.smsx.engine.service.group.datasource.JStoreGroupSendDataSource;
import ru.sibinco.smsx.engine.service.group.datasource.GroupEditDataSource;
import ru.sibinco.smsx.engine.service.group.datasource.DBGroupEditDataSource;
import ru.sibinco.smsx.engine.service.CommandExecutionException;
import ru.sibinco.smsx.engine.service.ServiceInitializationException;
import ru.sibinco.smsx.engine.service.Service;
import ru.sibinco.smsc.utils.admin.dl.DistributionList;
import ru.sibinco.smsc.utils.admin.dl.DistributionListManager;
import com.eyeline.utils.config.xml.XmlConfig;
import com.eyeline.utils.config.xml.XmlConfigSection;
import com.eyeline.sme.smpp.OutgoingQueue;

import java.util.List;

/**
 * User: artem
 * Date: 15.07.2008
 */

public class GroupServiceImpl implements Service, GroupService {

  private final GroupSendProcessor sendProcessor;
  private final GroupSendDataSource groupSendDS;
  private final GroupEditDataSource groupEditDS;
  private final GroupEditProcessor editProcessor;
  private final DistributionListManager dlManager;

  public GroupServiceImpl(XmlConfig config, OutgoingQueue outQueue) throws ServiceInitializationException {
    XmlConfigSection group = config.getSection("group");

    try {
      this.groupSendDS = new JStoreGroupSendDataSource(group.getString("store.file"), group.getInt("store.data.life.time"));
      this.sendProcessor = new GroupSendProcessor(outQueue, groupSendDS, group.getString("group.address"));

      this.dlManager = new DistributionListManager(group.getString("smsc.host"), group.getInt("smsc.port"), "windows-1251",
                                                   group.getInt("manager.pool.size"), group.getInt("manager.connection.timeout"));
      
      this.groupEditDS = new DBGroupEditDataSource();
      this.editProcessor = new GroupEditProcessor(outQueue, dlManager, groupEditDS, group.getString("source.address"), group.getSection("notifications").toProperties(""));
    } catch (Exception e) {
      throw new ServiceInitializationException(e);
    }
  }

  public void startService() {    
  }

  public void stopService() {
    groupSendDS.shutdown();
    groupEditDS.shutdown();    
  }

  public Object getMBean(String domain) {
    return new GroupMBean(domain, sendProcessor, dlManager);
  }

  public long execute(GroupSendCmd cmd) throws CommandExecutionException {
    return sendProcessor.execute(cmd);
  }

  public void execute(GroupDeliveryReportCmd cmd) throws CommandExecutionException {
    sendProcessor.execute(cmd);
  }

  public GroupSendStatusCmd.MessageStatus execute(GroupSendStatusCmd cmd) throws CommandExecutionException {
    return sendProcessor.execute(cmd);
  }

  public void execute(GroupAddCmd cmd) throws CommandExecutionException {
    editProcessor.execute(cmd);
  }

  public void execute(GroupRemoveCmd cmd) throws CommandExecutionException {
    editProcessor.execute(cmd);
  }

  public void execute(GroupRenameCmd cmd) throws CommandExecutionException {
    editProcessor.execute(cmd);
  }

  public void execute(GroupAddMemberCmd cmd) throws CommandExecutionException {
    editProcessor.execute(cmd);
  }

  public void execute(GroupRemoveMemberCmd cmd) throws CommandExecutionException {
    editProcessor.execute(cmd);
  }

  public GroupInfo execute(GroupInfoCmd cmd) throws CommandExecutionException {
    return editProcessor.execute(cmd);
  }

  public List<DistributionList> execute(GroupListCmd cmd) throws CommandExecutionException {
    return editProcessor.execute(cmd);
  }

  public void execute(GroupEditAlterProfileCmd cmd) throws CommandExecutionException {
    editProcessor.execute(cmd);
  }

  public void execute(GroupCopyCmd cmd) throws CommandExecutionException {
    editProcessor.execute(cmd);
  }
}
