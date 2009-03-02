package ru.sibinco.smsx.engine.service.group;

import com.eyeline.sme.smpp.OutgoingObject;
import com.eyeline.sme.smpp.OutgoingQueue;
import com.eyeline.sme.smpp.ShutdownedException;
import com.eyeline.utils.FixedArrayCache;
import com.eyeline.utils.ThreadFactoryWithCounter;
import com.eyeline.utils.config.ConfigException;
import com.eyeline.utils.config.xml.XmlConfigSection;
import org.apache.log4j.Category;
import ru.aurorisoft.smpp.Message;
import ru.aurorisoft.smpp.PDU;
import ru.sibinco.smsx.engine.service.AsyncCommand;
import ru.sibinco.smsx.engine.service.Command;
import ru.sibinco.smsx.engine.service.CommandExecutionException;
import ru.sibinco.smsx.engine.service.group.commands.*;
import ru.sibinco.smsx.engine.service.group.datasource.DistrList;
import ru.sibinco.smsx.engine.service.group.datasource.DistrListDataSource;
import ru.sibinco.smsx.engine.service.group.datasource.RepliesMap;
import ru.sibinco.smsx.utils.OperatorsList;

import java.util.ArrayList;
import java.util.concurrent.ArrayBlockingQueue;
import java.util.concurrent.ThreadPoolExecutor;
import java.util.concurrent.TimeUnit;

/**
 * User: artem
 * Date: 15.07.2008
 */

class GroupSendProcessor implements GroupSendCmd.Receiver,
    GroupSendStatusCmd.Receiver,
    GroupDeliveryReportCmd.Receiver,
    GroupReplyCmd.Receiver {

  private static final Category log = Category.getInstance("GROUP");

  private final OutgoingQueue outQueue;
  private final ThreadPoolExecutor executor;

  private final FixedArrayCache<DeliveryStatus[]> statuses;
  private final FixedArrayCache<DeliveryStatus> umrs;
  private final DistrListDataSource listsDS;
  private final RepliesMap replies;
  private final OperatorsList operators;

  private final String repliesInvintation;
  private final String sourceAddress;
  private final int serviceId;

  public GroupSendProcessor(XmlConfigSection sec, OutgoingQueue outQueue, DistrListDataSource listsDS, RepliesMap repliesMap, OperatorsList operators, int serviceId) throws ConfigException {
    this.outQueue = outQueue;
    this.repliesInvintation = sec.getString("reply.invitation");
    this.sourceAddress = sec.getString("source.address");
    this.listsDS = listsDS;
    this.replies = repliesMap;
    this.operators = operators;
    int statusesCacheSize = sec.getInt("statuses.cache.size", 10000);
    this.statuses = new FixedArrayCache<DeliveryStatus[]>(statusesCacheSize);
    this.umrs = new FixedArrayCache<DeliveryStatus>(statusesCacheSize);
    this.serviceId = serviceId;
    this.executor = new ThreadPoolExecutor(1, 10, 60L, TimeUnit.SECONDS, new ArrayBlockingQueue<Runnable>(100), new ThreadFactoryWithCounter("Group-Executor-"));
  }

  private long groupSend(GroupSendCommand cmd, DistrList dl) throws CommandExecutionException {
    try {
      if (dl == null)
        throw new CommandExecutionException("Group does not exists", GroupSendCmd.ERR_UNKNOWN_GROUP);

      if (!dl.containsSubmitter(cmd.getSubmitter()))
        throw new CommandExecutionException("Unknown submitter: " + cmd.getSubmitter(), GroupSendCmd.ERR_UNKNOWN_SUBMITTER);

      ArrayList<String> members = new ArrayList<String>(dl.members());
      members.remove(cmd.getSubmitter());

      DeliveryStatus[] deliveryStatuses = new DeliveryStatus[members.size()];
      int statusId = -1;
      if (cmd.isStorable())
        statusId = statuses.add(deliveryStatuses);

      if (!members.isEmpty()) {
        String replyInvintationText = repliesInvintation.replace("{size}", String.valueOf(members.size()));

        int i=0;
        for (String member : members) {

          Message m = new Message();
          m.setSourceAddress(cmd.getSubmitter());
          m.setDestinationAddress(member);
          m.setImsi(cmd.getImsi());
          m.setMscAddress(cmd.getMscAddress());
          m.setMessageString(cmd.getMessage());
          m.setDestAddrSubunit(cmd.getDestAddrSubunit());
          m.setConnectionName(cmd.getSourceId() == Command.SOURCE_SMPP ? "smsgroup" : "webgroup");

          DeliveryStatus dsm = new DeliveryStatus(member);
          deliveryStatuses[i] = dsm;
          GroupOutgoingObject o = new GroupOutgoingObject(cmd, deliveryStatuses, i);
          i++;

          if (cmd.isStorable()) {
            int umr = umrs.add(dsm);
            m.setUserMessageReference(umr * 10 + serviceId);
            m.setReceiptRequested(Message.RCPT_MC_FINAL_ALL);
          }

          o.setMessage(m);

          String operator = operators.getOperatorByAddress(member);
          if (dl.containsSubmitter(member) && operator != null) {
            replies.put(member, dl.getId());

            Message invintation = new Message();
            invintation.setSourceAddress(sourceAddress);
            invintation.setDestinationAddress(member);
            invintation.setMessageString(replyInvintationText);
            OutgoingObject o1 = new OutgoingObject();
            o1.setMessage(invintation);
            o.setReplyInvintation(o1);
          }

          outQueue.offer(o);
        }
      } else {
        cmd.setDeliveryStatuses(deliveryStatuses);
        cmd.update(AsyncCommand.STATUS_SUCCESS);
      }

      return statusId;
    } catch (CommandExecutionException e) {
      throw e;
    } catch (Throwable e) {
      log.error("GroupSendErr: owner=" + dl.getOwner() + "; name=" + dl.getName(), e);
      throw new CommandExecutionException("Error: " + e.getMessage(), GroupSendCmd.ERR_SYS_ERROR);
    }
  }

  public long execute(GroupSendCmd cmd) throws CommandExecutionException {
    if (log.isDebugEnabled())
      log.debug("GroupSendReq: owner=" + cmd.getOwner() + "; group=" + cmd.getGroupName());

    if (cmd.getOwner() == null || cmd.getOwner().trim().length() == 0)
      throw new CommandExecutionException("Owner is empty", GroupSendCmd.ERR_UNKNOWN_OWNER);

    if (cmd.getGroupName() == null || cmd.getGroupName().trim().length() == 0)
      throw new CommandExecutionException("Group name is empty", GroupSendCmd.ERR_UNKNOWN_GROUP);

    try {
      DistrList dl = listsDS.getDistrList(cmd.getGroupName(), cmd.getOwner());
      return groupSend(cmd, dl);
    } catch (CommandExecutionException e) {
      throw e;
    } catch (Throwable e) {
      log.error("GroupSendErr: owner=" + cmd.getOwner() + "; group=" + cmd.getGroupName(), e);
      throw new CommandExecutionException("Error: " + e.getMessage(), GroupSendCmd.ERR_SYS_ERROR);
    }
  }

  public long execute(GroupReplyCmd cmd) throws CommandExecutionException {
    if (log.isDebugEnabled())
      log.debug("GroupReplyReq: submitter=" + cmd.getSubmitter());

    try {
      int listId = replies.get(cmd.getSubmitter());
      if (listId == -1)
        throw new CommandExecutionException("Reply group for " + cmd.getSubmitter() + " not found.", GroupReplyCmd.ERR_UNKNOWN_GROUP);

      DistrList dl = listsDS.getDistrList(listId);
      return groupSend(cmd, dl);
    } catch (CommandExecutionException e) {
      throw e;
    } catch (Exception e) {
      log.error("GroupReplyErr: msisdn=" + cmd.getSubmitter(), e);
      throw new CommandExecutionException("Error: " + e.getMessage(), GroupSendCmd.ERR_SYS_ERROR);
    }
  }

  public DeliveryStatus[] execute(GroupSendStatusCmd cmd) throws CommandExecutionException {
    try {
      if (log.isDebugEnabled())
        log.debug("CheckStatusReq: msgId=" + cmd.getMsgId());

      return statuses.get(cmd.getMsgId());

    } catch (Throwable e) {
      log.error("CheckStatusErr: ", e);
      throw new CommandExecutionException("Error: " + e.getMessage(), GroupSendStatusCmd.ERR_SYS_ERROR);
    }
  }

  public boolean execute(GroupDeliveryReportCmd cmd) throws CommandExecutionException {
    try {
      if (log.isDebugEnabled())
        log.debug("DeliveryReportReq: umr=" + cmd.getUmr() + "; dlvrd=" + cmd.isDelivered());

      if (cmd.getUmr() % 10 == serviceId) {
        DeliveryStatus member = umrs.get(cmd.getUmr() / 10);
        if (member != null)
          member.status = cmd.isDelivered() ? DeliveryStatus.DELIVERED : DeliveryStatus.NOT_DELIVERED;
        else
          log.warn("No delivery found for umr=" + cmd.getUmr());
        return true;
      }

      return false;

    } catch (Throwable e) {
      log.error("DeliveryReportErr: ", e);
      throw new CommandExecutionException("Error: " + e.getMessage(), GroupDeliveryReportCmd.ERR_SYS_ERROR);
    }
  }

  public void shutdown() {
    executor.shutdownNow();
  }


  private class GroupOutgoingObject extends OutgoingObject {

    private final int index;
    private final DeliveryStatus[] statuses;
    private final GroupSendCommand cmd;
    private OutgoingObject replyInvintation;

    private GroupOutgoingObject(GroupSendCommand cmd, DeliveryStatus[] statuses, int index) {
      this.cmd = cmd;
      this.statuses = statuses;
      this.index = index;
    }

    public void setReplyInvintation(OutgoingObject replyInvintation) {
      this.replyInvintation = replyInvintation;
    }

    @Override
    protected void handleSendError() {
      statuses[index].status = DeliveryStatus.SYSTEM_ERROR;
    }

    @Override
    protected void handleResponse(final PDU pdu) {
      if (pdu.getStatusClass() == PDU.STATUS_CLASS_PERM_ERROR) {
        statuses[index].status = DeliveryStatus.NOT_DELIVERED;
      } else if (pdu.getStatusClass() == PDU.STATUS_CLASS_NO_ERROR) {
        statuses[index].status = DeliveryStatus.SENT;

        if (replyInvintation != null) {
          try {
            outQueue.offer(replyInvintation);
          } catch (ShutdownedException e) {
            log.error(e, e);
          }
        }
      }

      // Check all messages have been sent
      for (DeliveryStatus m : statuses) {
        if (m.status == DeliveryStatus.ACCEPTED)
          return;
      }

      // If all messages have been sent, update status
      try {
        executor.execute(new Runnable() {
          public void run() {
            cmd.setDeliveryStatuses(statuses);
            cmd.update(GroupSendCmd.STATUS_SUCCESS);
          }
        });
      } catch (Throwable e) {
        log.error(e, e);
      }
    }
  }
}
