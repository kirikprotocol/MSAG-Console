package ru.sibinco.smsx.engine.service.group;

import ru.sibinco.smsx.engine.service.group.commands.*;
import ru.sibinco.smsx.engine.service.group.datasource.GroupSendDataSource;
import ru.sibinco.smsx.engine.service.CommandExecutionException;
import ru.sibinco.smsx.utils.DataSourceException;
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

/**
 * User: artem
 * Date: 15.07.2008
 */

class GroupSendProcessor implements GroupSendCmd.Receiver,
                                GroupSendStatusCmd.Receiver,
                                GroupDeliveryReportCmd.Receiver {

  private static final Category log = Category.getInstance(GroupSendProcessor.class);

  private final OutgoingQueue outQueue;
  private final GroupSendDataSource ds;
  private final String groupServiceAddr;
  private final ThreadPoolExecutor executor;


  private final CycleNonblockingCounter umr = new CycleNonblockingCounter(0, 10000);
  private final NonblockingCounter rejectedTasks = new NonblockingCounter();

  public GroupSendProcessor(OutgoingQueue outQueue, GroupSendDataSource ds, String groupServiceAddr) {
    this.outQueue = outQueue;
    this.ds = ds;
    this.groupServiceAddr = groupServiceAddr;
    this.executor = new ThreadPoolExecutor(1, 10, 60L, TimeUnit.SECONDS, new ArrayBlockingQueue(100), new ThreadFactoryWithCounter("Group-Executor-"));
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
        ds.setStatus(msgKey, GroupSendDataSource.MessageStatus.ACCEPTED);
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

  public GroupSendStatusCmd.MessageStatus execute(GroupSendStatusCmd cmd) throws CommandExecutionException {
    try {
      if (log.isDebugEnabled())
        log.debug("CheckStatusReq: msgId=" + cmd.getMsgId());

      switch (ds.getStatus(cmd.getMsgId())) {
        case ACCEPTED: return GroupSendStatusCmd.MessageStatus.ACCEPTED;
        case DELIVERED: return GroupSendStatusCmd.MessageStatus.DELIVERED;
        case LIST_NOT_FOUND: return GroupSendStatusCmd.MessageStatus.LIST_NOT_FOUND;
        case OWNER_NOT_FOUND: return GroupSendStatusCmd.MessageStatus.OWNER_NOT_FOUND;
        case SYS_ERROR: return GroupSendStatusCmd.MessageStatus.SYS_ERR;
        default: return null;
      }

    } catch (Throwable e) {
      log.error("CheckStatusErr: ", e);
      throw new CommandExecutionException("Error: " + e.getMessage(), GroupSendStatusCmd.ERR_SYS_ERROR);
    }
  }

  public void execute(GroupDeliveryReportCmd cmd) throws CommandExecutionException {
    try {
      if (log.isDebugEnabled())
        log.debug("DeliveryReportReq: owner=" + cmd.getOwner() + "; umr=" + cmd.getUmr());

      GroupSendDataSource.MessageStatus status;
      switch (cmd.getDeliveryStatus()) {
        case DELIVERED: status = GroupSendDataSource.MessageStatus.DELIVERED; break;
        case LIST_NOT_FOUND: status = GroupSendDataSource.MessageStatus.LIST_NOT_FOUND; break;
        case OWNER_NOT_FOUND: status = GroupSendDataSource.MessageStatus.OWNER_NOT_FOUND; break;
        case ACCESS_DENIED: status = GroupSendDataSource.MessageStatus.SYS_ERROR; break;
        default: throw new CommandExecutionException("Unknown delivery status: " + cmd.getDeliveryStatus(), GroupDeliveryReportCmd.ERR_SYS_ERROR);
      }

      if (status != null)
        ds.setStatus(getMsgKey(cmd.getOwner(), cmd.getUmr()), status);

    } catch (Throwable e) {
      log.error("CheckStatusErr: ", e);
      throw new CommandExecutionException("Error: " + e.getMessage(), GroupDeliveryReportCmd.ERR_SYS_ERROR);
    }
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

    private void setError() {
      try {
        executor.execute(new Runnable() {
          public void run() {
            try {
              ds.setStatus(msgId, GroupSendDataSource.MessageStatus.SYS_ERROR);
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

    @Override
    protected void handleSendError() {
      setError();
    }

    @Override
    protected void handleResponse(final PDU pdu) {
      if (pdu.getStatusClass() == PDU.STATUS_CLASS_PERM_ERROR)
        setError();
    }
  }
}
