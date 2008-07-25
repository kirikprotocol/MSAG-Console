package ru.sibinco.smsx.engine.service.group;

import ru.sibinco.smsx.engine.service.group.commands.*;
import ru.sibinco.smsx.engine.service.group.datasource.GroupDataSource;
import ru.sibinco.smsx.engine.service.CommandExecutionException;
import ru.sibinco.smsx.utils.DataSourceException;
import ru.sibinco.smsc.utils.admin.dl.DistributionListManager;
import ru.sibinco.smsc.utils.admin.dl.DistributionList;
import ru.sibinco.smsc.utils.admin.dl.Principal;
import ru.sibinco.smsc.utils.admin.dl.exceptions.*;
import ru.sibinco.smsc.utils.admin.AdminException;
import ru.aurorisoft.smpp.Message;
import ru.aurorisoft.smpp.PDU;
import com.eyeline.sme.smpp.OutgoingQueue;
import com.eyeline.sme.smpp.OutgoingObject;
import com.eyeline.utils.NonblockingCounter;
import com.eyeline.utils.ThreadFactoryWithCounter;
import com.eyeline.utils.CycleNonblockingCounter;

import org.apache.log4j.Category;

import java.util.concurrent.ThreadPoolExecutor;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.ArrayBlockingQueue;
import java.util.List;

/**
 * User: artem
 * Date: 15.07.2008
 */

class GroupProcessor implements GroupSendCmd.Receiver,
                                CheckStatusCmd.Receiver,
                                GroupDeliveryReportCmd.Receiver,
                                GroupAddCmd.Receiver,
                                GroupRemoveCmd.Receiver,
                                GroupRenameCmd.Receiver,
                                GroupAddMemberCmd.Receiver,
                                GroupRemoveMemberCmd.Receiver,
                                GroupInfoCmd.Receiver,
                                GroupListCmd.Receiver {

  private static final Category log = Category.getInstance(GroupProcessor.class);

  private static final int MAX_GROUP_NAME_LEN = 18;

  private final OutgoingQueue outQueue;
  private final GroupDataSource ds;
  private final String groupServiceAddr;
  private final ThreadPoolExecutor executor;
  private final DistributionListManager dlmanager;

  private final CycleNonblockingCounter umr = new CycleNonblockingCounter(0, 10000);
  private final NonblockingCounter rejectedTasks = new NonblockingCounter();

  public GroupProcessor(OutgoingQueue outQueue, GroupDataSource ds, String groupServiceAddr, String smscHost, int smscPort) {
    this.outQueue = outQueue;
    this.ds = ds;
    this.groupServiceAddr = groupServiceAddr;
    this.dlmanager = new DistributionListManager(smscHost, smscPort, "windows-1251");
    this.executor = new ThreadPoolExecutor(1, 10, 60L, TimeUnit.SECONDS, new ArrayBlockingQueue(100), new ThreadFactoryWithCounter("Group-Executor-"));;
  }

  private static long getMsgKey(String owner, int usrMsgRef) {
    if (owner.charAt(0) == '+')
      owner = owner.substring(1);

    return Long.parseLong(owner) * 100000 + usrMsgRef;
  }

  public long execute(GroupSendCmd cmd) throws CommandExecutionException {
    if (log.isDebugEnabled())
      log.debug("GroupSendReq: owner=" + cmd.getOwner() + "; group=" + cmd.getGroupName());

    if (cmd.getOwner() == null || cmd.getOwner().trim().length() == 0)
      throw new CommandExecutionException("Owner is empty", GroupSendCmd.ERR_UNKNOWN_OWNER);

    if (cmd.getGroupName() == null || cmd.getGroupName().trim().length() == 0)
      throw new CommandExecutionException("Group name is empty", GroupSendCmd.ERR_UNKNOWN_GROUP);

    try {
      Message m = new Message();
      m.setSourceAddress(cmd.getOwner());
      m.setDestinationAddress(groupServiceAddr);
      m.setImsi(cmd.getImsi());
      m.setMscAddress(cmd.getMscAddress());
      m.setMessageString("SEND " + cmd.getGroupName() + ' ' + cmd.getMessage());
      m.setConnectionName("webgroup");

      OutgoingObject o;
      long msgKey = -1;
      if (cmd.isStorable()) {
        int umrVal;
        do {
          umrVal = umr.increment();
          msgKey = getMsgKey(cmd.getOwner(), umrVal);
        } while (ds.containsStatus(msgKey));
        m.setUserMessageReference(umrVal);        
        ds.setStatus(msgKey, GroupDataSource.MessageStatus.ACCEPTED);
        o = new GroupOutgoingObject(msgKey);
      } else
        o = new OutgoingObject();
      o.setMessage(m);

      outQueue.offer(o);

      return msgKey;
    } catch (Throwable e) {
      log.error("GroupSendErr: owner=" + cmd.getOwner() + "; group=" + cmd.getGroupName(), e);
      throw new CommandExecutionException("Error: " + e.getMessage(), GroupSendCmd.ERR_SYS_ERROR);
    }
  }

  public CheckStatusCmd.MessageStatus execute(CheckStatusCmd cmd) throws CommandExecutionException {
    try {
      if (log.isDebugEnabled())
        log.debug("CheckStatusReq: msgId=" + cmd.getMsgId());

      switch (ds.getStatus(cmd.getMsgId())) {
        case ACCEPTED: return CheckStatusCmd.MessageStatus.ACCEPTED;
        case DELIVERED: return CheckStatusCmd.MessageStatus.DELIVERED;
        case LIST_NOT_FOUND: return CheckStatusCmd.MessageStatus.LIST_NOT_FOUND;
        case OWNER_NOT_FOUND: return CheckStatusCmd.MessageStatus.OWNER_NOT_FOUND;
        case SYS_ERROR: return CheckStatusCmd.MessageStatus.SYS_ERR;
        default: return null;
      }

    } catch (Throwable e) {
      log.error("CheckStatusErr: ", e);
      throw new CommandExecutionException("Error: " + e.getMessage(), CheckStatusCmd.ERR_SYS_ERROR);
    }
  }

  public void execute(GroupDeliveryReportCmd cmd) throws CommandExecutionException {
    try {
      if (log.isDebugEnabled())
        log.debug("DeliveryReportReq: owner=" + cmd.getOwner() + "; umr=" + cmd.getUmr());

      GroupDataSource.MessageStatus status;
      switch (cmd.getDeliveryStatus()) {
        case DELIVERED: status = GroupDataSource.MessageStatus.DELIVERED; break;
        case LIST_NOT_FOUND: status = GroupDataSource.MessageStatus.LIST_NOT_FOUND; break;
        case OWNER_NOT_FOUND: status = GroupDataSource.MessageStatus.OWNER_NOT_FOUND; break;
        case ACCESS_DENIED: status = GroupDataSource.MessageStatus.SYS_ERROR; break;
        default: throw new CommandExecutionException("Unknown delivery status: " + cmd.getDeliveryStatus(), GroupDeliveryReportCmd.ERR_SYS_ERROR);
      }

      if (status != null)
        ds.setStatus(getMsgKey(cmd.getOwner(), cmd.getUmr()), status);

    } catch (Throwable e) {
      log.error("CheckStatusErr: ", e);
      throw new CommandExecutionException("Error: " + e.getMessage(), GroupDeliveryReportCmd.ERR_SYS_ERROR);
    }
  }

  private static void checkGroup(String groupName, String owner) throws CommandExecutionException {
    if (isEmpty(groupName) || groupName.length() > MAX_GROUP_NAME_LEN)
      throw new CommandExecutionException("Invalid group name", GroupCommand.ERR_INV_GROUP_NAME);

    if (isEmpty(owner))
      throw new CommandExecutionException("Invalid owner address", GroupCommand.ERR_INV_OWNER);
  }

  public void execute(GroupAddCmd cmd) throws CommandExecutionException {

    checkGroup(cmd.getGroupName(), cmd.getOwner());

    final DistributionList list = new DistributionList(cmd.getGroupName(), cmd.getOwner(), 50);
    for (int i=0; i<2; i++) {
      try {
        dlmanager.addDistributionList(list);
        return;

      } catch (PrincipalNotExistsException e) { // If principal does not exists, create it
        if (log.isDebugEnabled())
          log.debug("Principal " + cmd.getOwner() + " does not exists. Try to create it.");

        final Principal p = new Principal(cmd.getOwner(), 50, 50);
        try {
          dlmanager.addPrincipal(p);
        } catch (AdminException e1) {
          log.error("Principal creation error: ",e);
          throw new CommandExecutionException("Can't create principal", GroupAddCmd.ERR_SYS_ERROR);
        }

      } catch (AdminException e) {
        catchGroupException(e, cmd);
      }
    }
    throw new CommandExecutionException("Can't create group", GroupAddCmd.ERR_SYS_ERROR);
  }

  public void execute(GroupRemoveCmd cmd) throws CommandExecutionException {
    checkGroup(cmd.getGroupName(), cmd.getOwner());

    try {
      dlmanager.deleteDistributionList(cmd.getGroupName(), cmd.getOwner());
    } catch (AdminException e) {
      catchGroupException(e, cmd);
    }
  }

  public void execute(GroupRenameCmd cmd) throws CommandExecutionException {
    checkGroup(cmd.getGroupName(), cmd.getOwner());
    checkGroup(cmd.getNewGroupName(), cmd.getOwner());

    try {
      dlmanager.renameDistributionList(cmd.getGroupName(), cmd.getOwner(), cmd.getNewGroupName());
    } catch (AdminException e) {
      catchGroupException(e, cmd);
    }
  }

  public void execute(GroupAddMemberCmd cmd) throws CommandExecutionException {
    checkGroup(cmd.getGroupName(), cmd.getOwner());
    if (isEmpty(cmd.getMember()))
      throw new CommandExecutionException("Invalid member msisdn", GroupAddMemberCmd.ERR_INV_MEMBER);

    try {
      dlmanager.addMember(cmd.getGroupName(), cmd.getOwner(), cmd.getMember());
    } catch (AdminException e) {
      catchGroupException(e, cmd);
    }
  }

  public void execute(GroupRemoveMemberCmd cmd) throws CommandExecutionException {
    checkGroup(cmd.getGroupName(), cmd.getOwner());
    if (isEmpty(cmd.getMember()))
      throw new CommandExecutionException("Invalid member msisdn", GroupAddMemberCmd.ERR_INV_MEMBER);

    try {
      dlmanager.deleteMember(cmd.getGroupName(), cmd.getOwner(), cmd.getMember());
    } catch (AdminException e) {
      catchGroupException(e, cmd);
    }
  }

  public GroupInfo execute(GroupInfoCmd cmd) throws CommandExecutionException {
    checkGroup(cmd.getGroupName(), cmd.getOwner());

    try {
      List<String> members = dlmanager.members(cmd.getGroupName(), cmd.getOwner());
      return new GroupInfo(members);
    } catch (AdminException e) {
      catchGroupException(e, cmd);
    }
    return null;
  }

  public List<DistributionList> execute(GroupListCmd cmd) throws CommandExecutionException {
    if (isEmpty(cmd.getOwner()))
      throw new CommandExecutionException("Invalid owner msisdn", GroupCommand.ERR_INV_OWNER);

    try {
      return dlmanager.list(cmd.getOwner());
    } catch (AdminException e) {
      catchGroupException(e, cmd);
    }    
    return null;
  }

  private static void catchGroupException(AdminException ex, GroupCommand cmd) throws CommandExecutionException {
    try {
      log.error("Cmd execution failed: owner=" + cmd.getOwner() + "; dl=" + cmd.getGroupName());
      throw ex;
    } catch (ListNotExistsException e) {
      throw new CommandExecutionException(e.getMessage(), GroupCommand.ERR_GROUP_NOT_EXISTS);
    } catch (MemberAlreadyExistsException e) {
      throw new CommandExecutionException(e.getMessage(), GroupCommand.ERR_MEMBER_ALREADY_EXISTS);
    } catch (MembersCountExceededException e) {
      throw new CommandExecutionException(e.getMessage(), GroupCommand.ERR_MEMBER_COUNT_EXCEEDED);
    } catch (PrincipalNotExistsException e) {
      throw new CommandExecutionException(e.getMessage(), GroupCommand.ERR_OWNER_NOT_EXISTS);
    } catch (ListAlreadyExistsException e) {
      throw new CommandExecutionException(e.getMessage(), GroupCommand.ERR_GROUP_ALREADY_EXISTS);
    } catch (ListsCountExceededException e) {
      throw new CommandExecutionException(e.getMessage(), GroupCommand.ERR_GROUPS_COUNT_EXCEEDED);
    } catch (MemberNotExistsException e) {
      throw new CommandExecutionException(e.getMessage(), GroupCommand.ERR_MEMBER_NOT_EXISTS);
    } catch (AdminException e) {
      throw new CommandExecutionException(e.getMessage(), GroupAddMemberCmd.ERR_SYS_ERROR);
    }
  }

  private static boolean isEmpty(String str) {
    return str == null || str.trim().length() == 0;
  }

  public int getExecutorActiveCount() {
    return executor.getActiveCount();
  }

  public int getExecutorPoolSize() {
    return executor.getPoolSize();
  }

  public int getExecutorMaxPoolSize() {
    return executor.getMaximumPoolSize();
  }

  public void setExecutorMaxPoolSize(int size) {
    executor.setMaximumPoolSize(size);
  }

  public int getExecutorRejectedTasks() {
    return rejectedTasks.getValue();
  }


  private class GroupOutgoingObject extends OutgoingObject {

    private final long msgId;

    private GroupOutgoingObject(long msgId) {
      this.msgId = msgId;
    }

    @Override
    protected void handleResponse(final PDU pdu) {
      try {
        executor.execute(new Runnable() {
          public void run() {
            try {
              if (pdu.getStatusClass() == PDU.STATUS_CLASS_PERM_ERROR)
                ds.setStatus(msgId, GroupDataSource.MessageStatus.SYS_ERROR);
            } catch (DataSourceException e) {
              log.error(e,e);
            }
          }
        });
      } catch (Throwable e) {
        log.error(e,e);
        rejectedTasks.increment();
      }
    }
  }
}
