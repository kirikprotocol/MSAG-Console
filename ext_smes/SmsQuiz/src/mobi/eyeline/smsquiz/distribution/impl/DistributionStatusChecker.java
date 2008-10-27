package mobi.eyeline.smsquiz.distribution.impl;

import org.apache.log4j.Logger;

import java.util.Map;

import mobi.eyeline.smsquiz.distribution.smscconsole.SmscConsoleResponse;
import mobi.eyeline.smsquiz.distribution.smscconsole.SmscConsoleException;
import mobi.eyeline.smsquiz.distribution.smscconsole.SmscConsoleClient;

/**
 * author: alkhal
 */
public class DistributionStatusChecker implements Runnable {

  private Map<String, Status> tasksMap;
  private long maxWait;
  private SmscConsoleClient consoleClient;
  private String statusCommand;
  private String codeOk;
  private String successStatus;

  private static Logger logger = Logger.getLogger(DistributionStatusChecker.class);

  public DistributionStatusChecker(Map<String, Status> tasksMap, long maxWait,
                                   String statusCommand, String codeOk, String successStatus, SmscConsoleClient consoleClient) {
    if ((tasksMap == null) || (statusCommand == null) || (codeOk == null) || (successStatus == null) || (consoleClient == null)) {
      throw new NullPointerException("Some params are null");
    }
    this.tasksMap = tasksMap;
    this.maxWait = maxWait;
    this.consoleClient = consoleClient;
    this.statusCommand = statusCommand;
    this.codeOk = codeOk;
    this.successStatus = successStatus;
  }

  public void run() {
    logger.info("DistributionStatusChecker starts...");
    try {
      SmscConsoleResponse response;
      for (Map.Entry<String, Status> entry : tasksMap.entrySet()) {
        String id = entry.getKey();
        if (logger.isInfoEnabled()) {
          logger.info("Get status for id=" + id);
        }
        StringBuilder command = new StringBuilder();
        command.append(statusCommand);
        command.append(" \"").append(id).append("\"");

        if (logger.isInfoEnabled()) {
          logger.info("Sending console command: " + command.toString());
        }

        response = consoleClient.sendCommand(command.toString());
        if ((response != null) && (response.isSuccess()) && (response.getStatus().trim().equals(codeOk))) {
          String[] lines = response.getLines();
          if (lines.length > 0) {
            String[] tokens = lines[0].trim().split(" ");
            if (tokens.length < 3) {
              throw new SmscConsoleException("Wrong response");
            }
            String status = tokens[2];
            handle(id, status, entry.getValue());
          } else {
            throw new SmscConsoleException("Wrong response: response line is empty");
          }
        } else {
          throw new SmscConsoleException("Wrong response");
        }
      }
      logger.info("DistributionStatusChecker finished.");
    } catch (SmscConsoleException e) {
      logger.error("Error during communicating", e);
      e.printStackTrace();
    }
  }

  private void handle(String id, String status, Status task) {
    if (logger.isInfoEnabled()) {
      logger.info("Handling: id=" + id + " status=" + status);
    }
    if (status.equals(successStatus)) {
      if (!task.isExecuted()) {
        task.setStatus(Status.DistrStatus.FINISHED);
        task.start();
        tasksMap.remove(id);
        logger.info("Distribution generated, external task begins...");
      }
    } else {
      long creationDate = task.getCreationDate();
      if ((System.currentTimeMillis() - creationDate) > maxWait) {
        tasksMap.remove(id);
        task.generateErrorFile();
        if (logger.isInfoEnabled()) {
          logger.warn("Time for waiting distribution is expired. Tasks observing deleted for id=" + id);
        }
      }
    }
  }
}
