package ru.sibinco.smpp.cmb;

import com.logica.smpp.Data;
import ru.aurorisoft.smpp.Message;
import ru.sibinco.smpp.*;

import java.io.IOException;
import java.text.MessageFormat;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Properties;

/**
 * Created by IntelliJ IDEA.
 * User: kpv
 * Date: Dec 21, 2006
 * Time: 6:46:25 PM
 */
public class CallMeBackProcessor implements RequestProcessor {

  protected static org.apache.log4j.Category Logger = org.apache.log4j.Category.getInstance(CallMeBackProcessor.class);

  private String name = "CallMeBackProcessor";
  private ConstraintManager constraintManager = null;
  private ReportsQueue rQueue = ReportsQueue.getInstance();
  private int id = 1;

  private String listenerName = "ReportSender";

  private String balanceLimit = "Usluga dostupna tolko abobentam v chastichnoy blokirovke.";
  private String usageLimit = "Kolichestvo zaprosov na segodnja ischerpano. Maksimalno Ð {0} zaprosov v den'.";
  private String attemptsLimit = "Kolichestvo popytok na segodnja ischerpano. Maksimalno Ð {0} popytok v den'.";
  private String notification = "Pozhaluysta, perezvonite mne {0}. Otpravleno {1}";
  private String confirmation = "Vash zapros prinjat. U vas ostalos {0} zaprosov na segodnja.";
  private String failed = "V dannyj moment usluga ne dostupna. Poprobujte povtorit' zapros pozzhe.";
  private String invalid = "Nepravilniy format zaprosa ili nomer abonenta. Nabirajte *{0}*XXXXXXX#. U vas ostalos {1} popytok otpavit zapros segodnya.";
  private String successReport = "Soobschenie abonentu {0} otpravleno.";
  private String failedReport = "Soobschenie abonentu {0} ne dostavleno. U Vas ostalos' {1} popytok dostavki.";

  private SimpleDateFormat sdf = new SimpleDateFormat("HH:mm dd/MM/yy");

  public void init(String name, String fileName) throws InitializationException {
    if (name != null && !(name.trim().length() == 0))
      this.name = name;
    Properties config = new Properties();
    try {
      config.load(this.getClass().getClassLoader().getResourceAsStream(fileName));
    } catch (IOException e) {
      throw new InitializationException("Could not load " + this.name + " config from " + fileName, e);
    }
    init(name, config);
  }

  public void init(String name, Properties config) throws InitializationException {
    if (config.getProperty("constraint.manager.class") != null) {
      try {
        constraintManager = (ConstraintManager) Class.forName(config.getProperty("constraint.manager.class")).newInstance();
      } catch (Exception e) {
        throw new InitializationException("Could not initialize ConstraintManager " + config.getProperty("constraint.manager.class") + " for " + this.name, e);
      }
    } else {
      constraintManager = new ConstraintManagerImpl();
    }

    listenerName = config.getProperty("notification.status.listener", listenerName);
    Logger.info("\"notification.status.listener\"=" + listenerName);

    balanceLimit = config.getProperty("balance.limit.message", balanceLimit);
    Logger.info("\"balance.limit.message\"=" + balanceLimit);

    usageLimit = config.getProperty("usages.limit.message", usageLimit);
    Logger.info("\"usages.limit.message\"=" + usageLimit);

    attemptsLimit = config.getProperty("attempts.limit.message", attemptsLimit);
    Logger.info("\"attempts.limit.message\"=" + attemptsLimit);

    notification = config.getProperty("notification.message", notification);
    Logger.info("\"notification.message\"=" + notification);

    confirmation = config.getProperty("confirmation.message", confirmation);
    Logger.info("\"confirmation.message\"=" + confirmation);

    failed = config.getProperty("failed.message", failed);
    Logger.info("\"failed.message\"=" + failed);

    invalid = config.getProperty("invalid.message", invalid);
    Logger.info("\"invalid.message\"=" + invalid);

    successReport = config.getProperty("success.report.message", successReport);
    Logger.info("\"success.report.message\"=" + successReport);

    failedReport = config.getProperty("failed.report.message", failedReport);
    Logger.info("\"failed.report.message\"=" + failedReport);
  }

  public String getName() {
    return name;
  }

  public Response process(MessageData messageData) throws RequestProcessingException {
    if (!messageData.hasUssdServiceOp() ||
        messageData.getUssdServiceOp() != MessageData.USSD_OP_PROC_SS_REQ_IND)
      throw new RequestProcessingException("Could not process message with ussd_service_op=" + messageData.getUssdServiceOp());
    try {
      // get data from cache
      constraintManager.check(messageData.getSourceAddress());
      // update data
      int a =  constraintManager.registerAttempt(messageData.getSourceAddress());
      int u = constraintManager.registerUsage(messageData.getSourceAddress());
      if (messageData.getMessageString() == null || messageData.getMessageString().trim().length() != 7) {
        Logger.warn("Invalid request " + messageData.getMessageString());
        return prepareResponse(messageData, MessageFormat.format(invalid, new Object[]{(messageData.getDestinationAddress().indexOf(":") > 0 ? messageData.getDestinationAddress().substring(messageData.getDestinationAddress().indexOf(":") + 1) : messageData.getDestinationAddress()), Integer.toString(a)}));
      } else {
        try {
          Integer.parseInt(messageData.getMessageString());
          // prepare confirmatoin & notification
          Response response = prepareResponse(messageData, MessageFormat.format(confirmation, new Object[]{Integer.toString(u)}));
          MessageData n = new MessageData();
          n.setId(getNextId());
          n.setMessageStatusListenerName(listenerName);
          n.setSourceAddress(messageData.getSourceAddress());
          n.setDestinationAddress("+37529" + messageData.getMessageString().trim());
          n.setEsmClass((byte) Data.SM_FORWARD_MODE);
          n.setType(Message.TYPE_SUBMIT);
          n.setMessageString(MessageFormat.format(notification, new Object[]{messageData.getSourceAddress(), sdf.format(Calendar.getInstance().getTime())}));
          response.addMessage(n);
          // prepare reports
          MessageData sr = prepareResponseMessage(messageData, MessageFormat.format(successReport, new Object[]{n.getDestinationAddress()}));
          MessageData fr = prepareResponseMessage(messageData, MessageFormat.format(failedReport, new Object[]{n.getDestinationAddress(), Integer.toString(a)}));
          rQueue.addReport(new Report(n.getId(), sr, fr));
          // return response
          return response;
        } catch (NumberFormatException e) {
          Logger.warn("Invalid request " + messageData.getMessageString());
          constraintManager.registerAttempt(messageData.getSourceAddress());
          return prepareResponse(messageData, MessageFormat.format(invalid, new Object[]{(messageData.getDestinationAddress().indexOf(":") > 0 ? messageData.getDestinationAddress().substring(messageData.getDestinationAddress().indexOf(":") + 1) : messageData.getDestinationAddress()), Integer.toString(a)}));
        }
      }
    } catch (CheckConstraintsException e) {
      Logger.warn(e.getMessage(), e);
      return prepareResponse(messageData, failed);
    } catch (AttemptsLimitReachedException e) {
      Logger.warn(e.getMessage(), e);
      return prepareResponse(messageData, MessageFormat.format(attemptsLimit, new Object[]{Integer.toString(e.getLimit())}));
    } catch (UsageLimitReachedException e) {
      Logger.warn(e.getMessage(), e);
      return prepareResponse(messageData, MessageFormat.format(usageLimit, new Object[]{Integer.toString(e.getLimit())}));
    } catch (BalanceLimitException e) {
      Logger.warn(e.getMessage(), e);
      return prepareResponse(messageData, balanceLimit);
    }
  }

  private Response prepareResponse(MessageData messageData, String answer) {
    MessageData resp = new MessageData(messageData);
    String oa = resp.getSourceAddress();
    resp.setSourceAddress(resp.getDestinationAddress());
    resp.setDestinationAddress(oa);
    resp.setUssdServiceOp(MessageData.USSD_OP_PROC_SS_REQ_RESP);
    resp.setMessageString(answer);
    return new Response(Response.TYPE_ARRAY, new MessageData[]{resp});
  }

  private MessageData prepareResponseMessage(MessageData messageData, String answer) {
    MessageData resp = new MessageData(messageData);
    String oa = resp.getSourceAddress();
    resp.setUssdServiceOp(-1);
    resp.setSourceAddress(resp.getDestinationAddress());
    resp.setDestinationAddress(oa);
    resp.setMessageString(answer);
    resp.setEsmClass((byte) Data.SM_FORWARD_MODE);
    resp.setType(Message.TYPE_SUBMIT);
    return resp;
  }

  private int getNextId() {
    if (id == Integer.MAX_VALUE) {
      id = 0;
    }
    return id++;
  }
}
