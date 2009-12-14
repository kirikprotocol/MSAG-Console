package ru.sibinco.smsx.engine.service.group;

import com.eyeline.sme.smpp.OutgoingQueue;
import com.eyeline.utils.config.xml.XmlConfigSection;
import ru.sibinco.smsx.engine.service.CommandExecutionException;
import ru.sibinco.smsx.engine.service.Service;
import ru.sibinco.smsx.engine.service.ServiceInitializationException;
import ru.sibinco.smsx.engine.service.group.commands.*;
import ru.sibinco.smsx.engine.service.group.datasource.*;
import ru.sibinco.smsx.engine.service.group.datasource.impl.distrlist.FileDistrListDataSource;
import ru.sibinco.smsx.engine.service.group.datasource.impl.distrlist.DBDistrListDataSource;
import ru.sibinco.smsx.engine.service.group.datasource.impl.principal.FilePrincipalDataSource;
import ru.sibinco.smsx.engine.service.group.datasource.impl.principal.DBPrincipalDataSource;
import ru.sibinco.smsx.engine.service.group.datasource.impl.profile.DBGroupEditProfileDataSource;
import ru.sibinco.smsx.engine.service.group.datasource.impl.send.DBGroupSendDataSource;
import ru.sibinco.smsx.Context;

import java.io.File;
import java.util.Collection;

/**
 * User: artem
 * Date: 15.07.2008
 */

public class GroupServiceImpl implements Service, GroupService {

  private final GroupSendProcessor sendProcessor;
  private final GroupEditProfileDataSource groupEditDSProfile;
  private final GroupEditProcessor editProcessor;
  private final DistrListDataSource listsDS;
  private final PrincipalDataSource principalDS;
  private final RepliesMap replies;
  private final GroupProfileProcessor profileProcessor;
  private final GroupSendDataSource sendDS;

  public GroupServiceImpl(XmlConfigSection g, OutgoingQueue outQueue, int serviceId) throws ServiceInitializationException {
    try {
      String storeDir = g.getString("store.dir");
      this.listsDS = new FileDistrListDataSource(new File(storeDir, "members.bin"), new File(storeDir, "submitters.bin"), new File(storeDir, "lists.bin"));
//      this.listsDS = new DBDistrListDataSource();
      this.principalDS = new FilePrincipalDataSource(new File(storeDir, "principals.bin"));
//      this.principalDS = new DBPrincipalDataSource();
      this.groupEditDSProfile = new DBGroupEditProfileDataSource();
      this.sendDS = new DBGroupSendDataSource();

      this.replies = new RepliesMap(new File(storeDir, "groupreplies.bin"), g.getInt("replies.cache.size", 10000));
      this.sendProcessor = new GroupSendProcessor(g, outQueue, listsDS, replies, Context.getInstance().getOperators(), sendDS, serviceId);
      this.editProcessor = new GroupEditProcessor(g, listsDS, principalDS, Context.getInstance().getOperators());
      this.profileProcessor = new GroupProfileProcessor(groupEditDSProfile);
    } catch (Exception e) {
      throw new ServiceInitializationException(e);
    }
  }

  public void startService() {    
  }

  public void stopService() {
    sendProcessor.shutdown();
    groupEditDSProfile.shutdown();
    listsDS.close();
    principalDS.close();
    replies.shutdown();
    sendDS.release();
  }

  public Object getMBean(String domain) {
    return new GroupMBean(replies, listsDS);
  }

  public long execute(GroupSendCmd cmd) throws CommandExecutionException {
    return sendProcessor.execute(cmd);
  }

  public long execute(GroupReplyCmd cmd) throws CommandExecutionException {
    return sendProcessor.execute(cmd);
  }

  public boolean execute(GroupDeliveryReportCmd cmd) throws CommandExecutionException {
    return sendProcessor.execute(cmd);
  }

  public DeliveryStatus[] execute(GroupSendStatusCmd cmd) throws CommandExecutionException {
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

  public Collection<DistrList> execute(GroupListCmd cmd) throws CommandExecutionException {
    return editProcessor.execute(cmd);
  }

  public void execute(GroupEditAlterProfileCmd cmd) throws CommandExecutionException {
    profileProcessor.execute(cmd);
  }

  public void execute(GroupCopyCmd cmd) throws CommandExecutionException {
    editProcessor.execute(cmd);
  }

  public GroupEditGetProfileCmd.Result execute(GroupEditGetProfileCmd cmd) throws CommandExecutionException {
    return profileProcessor.execute(cmd);
  }

  public void execute(GroupAddSubmitterCmd cmd) throws CommandExecutionException {
    editProcessor.execute(cmd);
  }

  public void execute(GroupRemoveSubmitterCmd cmd) throws CommandExecutionException {
    editProcessor.execute(cmd);
  }

  public void execute(PrincipalAddCmd cmd) throws CommandExecutionException {
    editProcessor.execute(cmd);
  }

  public void execute(PrincipalRemoveCmd cmd) throws CommandExecutionException {
    editProcessor.execute(cmd);
  }

  public Collection<Principal> execute(PrincipalListCmd cmd) throws CommandExecutionException {
    return editProcessor.execute(cmd);
  }

  public Principal execute(PrincipalGetCmd cmd) throws CommandExecutionException {
    return editProcessor.execute(cmd);
  }

  public void execute(PrincipalAlterCmd cmd) throws CommandExecutionException {
    editProcessor.execute(cmd);
  }

  public void execute(GroupAlterCmd cmd) throws CommandExecutionException {
    editProcessor.execute(cmd);
  }
}
