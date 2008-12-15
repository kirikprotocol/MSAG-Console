package ru.sibinco.smsx.engine.service.group;

import com.eyeline.sme.smpp.OutgoingObject;
import com.eyeline.sme.smpp.OutgoingQueue;
import com.eyeline.utils.FixedArrayCache;
import com.eyeline.utils.ThreadFactoryWithCounter;
import com.eyeline.utils.config.xml.XmlConfigSection;
import com.eyeline.utils.config.ConfigException;
import org.apache.log4j.Category;
import ru.aurorisoft.smpp.Message;
import ru.aurorisoft.smpp.PDU;
import ru.sibinco.smsx.engine.service.Command;
import ru.sibinco.smsx.engine.service.CommandExecutionException;
import ru.sibinco.smsx.engine.service.group.commands.*;
import ru.sibinco.smsx.engine.service.group.datasource.DistrList;
import ru.sibinco.smsx.engine.service.group.datasource.DistrListDataSource;
import ru.sibinco.smsx.engine.service.group.datasource.RepliesMap;
import ru.sibinco.smsx.utils.OperatorsList;

import java.util.concurrent.ArrayBlockingQueue;
import java.util.concurrent.ThreadPoolExecutor;
import java.util.concurrent.TimeUnit;
import java.util.Collection;

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

  private final FixedArrayCache<DeliveryStatus> statuses;
  private final DistrListDataSource listsDS;
  private final RepliesMap replies;
  private final OperatorsList operators;

  private final String repliesInvintation;
  private final String sourceAddress;

  public GroupSendProcessor(XmlConfigSection sec, OutgoingQueue outQueue, DistrListDataSource listsDS, RepliesMap repliesMap, OperatorsList operators) throws ConfigException {
    this.outQueue = outQueue;
    this.repliesInvintation = sec.getString("reply.invitation");
    this.sourceAddress = sec.getString("source.address");
    this.listsDS = listsDS;
    this.replies = repliesMap;
    this.operators = operators;
    this.statuses = new FixedArrayCache<DeliveryStatus>(sec.getInt("statuses.cache.size"));
    this.executor = new ThreadPoolExecutor(1, 10, 60L, TimeUnit.SECONDS, new ArrayBlockingQueue<Runnable>(100), new ThreadFactoryWithCounter("Group-Executor-"));
  }

  private long groupSend(GroupSendCommand cmd, DistrList dl) throws CommandExecutionException {
    try {
      if (dl == null)
        throw new CommandExecutionException("Group does not exists", GroupSendCmd.ERR_UNKNOWN_GROUP);

      if (!dl.containsSubmitter(cmd.getSubmitter()))
        throw new CommandExecutionException("Unknown submitter: " + cmd.getSubmitter(), GroupSendCmd.ERR_UNKNOWN_SUBMITTER);

      DeliveryStatus s = new DeliveryStatus(dl.members());
      int umr = -1;
      if (cmd.isStorable())
        umr = statuses.add(s);

      final Collection<String> members = dl.members();
      if (!members.isEmpty()) {
        String replyInvintationText = repliesInvintation.replace("{size}", String.valueOf(members.size()));

        for (String member : members) {
          if (member.equals(cmd.getSubmitter()))
            continue;
        
          Message m = new Message();
          m.setSourceAddress(cmd.getSubmitter());
          m.setDestinationAddress(member);
          m.setImsi(cmd.getImsi());
          m.setMscAddress(cmd.getMscAddress());
          m.setMessageString(cmd.getMessage());
          m.setDestAddrSubunit(cmd.getDestAddrSubunit());
          if (cmd.getSourceId() == Command.SOURCE_SMPP)
            m.setConnectionName("smsgroup");
          else
            m.setConnectionName("webgroup");

          OutgoingObject o = new GroupOutgoingObject(cmd, s);
          if (cmd.isStorable()) {
            m.setUserMessageReference(umr);
            m.setReceiptRequested(Message.RCPT_MC_FINAL_ALL);
          }
          o.setMessage(m);

          OutgoingObject o1 = null;
          String operator = operators.getOperatorByAddress(member);
          if (dl.containsSubmitter(member) && operator != null) {
            replies.put(member, dl.getId());

            if (!member.equals(cmd.getSubmitter())) {
              Message invintation = new Message();
              invintation.setSourceAddress(sourceAddress);
              invintation.setDestinationAddress(member);
              invintation.setMessageString(replyInvintationText);
              o1 = new OutgoingObject();
              o1.setMessage(invintation);
            }
          }

          outQueue.offer(o);
          if (o1 != null)
            outQueue.offer(o1);
        }
      }

      return umr;
    } catch (CommandExecutionException e) {
      throw e;
    } catch (Throwable e) {
      log.error("GroupSendErr: owner=" + dl.getOwner() + "; name=" + dl.getName() , e);
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

  public DeliveryStatus execute(GroupSendStatusCmd cmd) throws CommandExecutionException {
    try {
      if (log.isDebugEnabled())
        log.debug("CheckStatusReq: msgId=" + cmd.getMsgId());

      return statuses.get(cmd.getMsgId());

    } catch (Throwable e) {
      log.error("CheckStatusErr: ", e);
      throw new CommandExecutionException("Error: " + e.getMessage(), GroupSendStatusCmd.ERR_SYS_ERROR);
    }
  }

  public void execute(GroupDeliveryReportCmd cmd) throws CommandExecutionException {
    try {
      if (log.isDebugEnabled())
        log.debug("DeliveryReportReq: da=" + cmd.getAddress() + "; umr=" + cmd.getUmr() + "; dlvrd=" + cmd.isDelivered());

      DeliveryStatus s = statuses.get(cmd.getUmr());
      if (s != null) {
        s.setStatus(cmd.getAddress(), cmd.isDelivered() ? DeliveryStatus.DELIVERED : DeliveryStatus.NOT_DELIVERED);
      } else
        log.warn("No delivery found for umr=" + cmd.getUmr());

    } catch (Throwable e) {
      log.error("DeliveryReportErr: ", e);
      throw new CommandExecutionException("Error: " + e.getMessage(), GroupDeliveryReportCmd.ERR_SYS_ERROR);
    }
  }

  public void shutdown() {
    executor.shutdownNow();
  }


  private class GroupOutgoingObject extends OutgoingObject {

    private final DeliveryStatus s;
    private final GroupSendCommand cmd;

    private GroupOutgoingObject(GroupSendCommand cmd, DeliveryStatus s) {
      this.cmd = cmd;
      this.s = s;
    }

    @Override
    protected void handleSendError() {
      s.setStatus(getMessage().getDestinationAddress(), DeliveryStatus.NOT_DELIVERED);
    }

    @Override
    protected void handleResponse(final PDU pdu) {
      if (pdu.getStatusClass() == PDU.STATUS_CLASS_PERM_ERROR)
        s.setStatus(getMessage().getDestinationAddress(), DeliveryStatus.NOT_DELIVERED);
      else {
        s.setStatus(getMessage().getDestinationAddress(), DeliveryStatus.SENT);
        if (s.isAllMessagesSent()) {
          try {
            executor.execute(new Runnable() {
              public void run() {
                cmd.setDeliveryStatus(s);
                cmd.update(GroupSendCmd.STATUS_SUCCESS);
              }
            });
          } catch (Throwable e) {
            log.error(e,e);
          }
        }
      }
    }
  }
}
