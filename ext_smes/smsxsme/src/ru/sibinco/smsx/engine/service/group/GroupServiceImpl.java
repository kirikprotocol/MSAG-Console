package ru.sibinco.smsx.engine.service.group;

import ru.sibinco.smsx.engine.service.group.commands.*;
import ru.sibinco.smsx.engine.service.group.datasource.GroupDataSource;
import ru.sibinco.smsx.engine.service.group.datasource.JStoreGroupDataSource;
import ru.sibinco.smsx.engine.service.CommandExecutionException;
import ru.sibinco.smsx.engine.service.ServiceInitializationException;
import ru.sibinco.smsx.engine.service.Service;
import ru.sibinco.smsc.utils.admin.dl.DistributionList;
import com.eyeline.utils.config.xml.XmlConfig;
import com.eyeline.utils.config.xml.XmlConfigSection;
import com.eyeline.sme.smpp.OutgoingQueue;

import java.util.concurrent.ExecutorService;
import java.util.List;

/**
 * User: artem
 * Date: 15.07.2008
 */

public class GroupServiceImpl implements Service, GroupService {

  private final GroupProcessor processor;
  private final GroupDataSource ds;

  public GroupServiceImpl(XmlConfig config, OutgoingQueue outQueue) throws ServiceInitializationException {
    XmlConfigSection group = config.getSection("group");

    try {
      this.ds = new JStoreGroupDataSource(group.getString("store.file"), group.getInt("store.data.life.time"));
      this.processor = new GroupProcessor(outQueue, ds, group.getString("group.address"), group.getString("smsc.host"), group.getInt("smsc.port"));
    } catch (Exception e) {
      throw new ServiceInitializationException(e);
    }
  }

  public void startService() {    
  }

  public void stopService() {
    ds.shutdown();
  }

  public Object getMBean(String domain) {
    return new GroupMBean(processor);
  }

  public long execute(GroupSendCmd cmd) throws CommandExecutionException {
    return processor.execute(cmd);
  }

  public void execute(GroupDeliveryReportCmd cmd) throws CommandExecutionException {
    processor.execute(cmd);
  }

  public CheckStatusCmd.MessageStatus execute(CheckStatusCmd cmd) throws CommandExecutionException {
    return processor.execute(cmd);
  }

  public void execute(GroupAddCmd cmd) throws CommandExecutionException {
    processor.execute(cmd);
  }

  public void execute(GroupRemoveCmd cmd) throws CommandExecutionException {
    processor.execute(cmd);
  }

  public void execute(GroupRenameCmd cmd) throws CommandExecutionException {
    processor.execute(cmd);
  }

  public void execute(GroupAddMemberCmd cmd) throws CommandExecutionException {
    processor.execute(cmd);
  }

  public void execute(GroupRemoveMemberCmd cmd) throws CommandExecutionException {
    processor.execute(cmd);
  }

  public GroupInfo execute(GroupInfoCmd cmd) throws CommandExecutionException {
    return processor.execute(cmd);
  }

  public List<DistributionList> execute(GroupListCmd cmd) throws CommandExecutionException {
    return processor.execute(cmd);
  }
}
