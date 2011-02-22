package mobi.eyeline.informer.admin.archive;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.*;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import org.apache.log4j.Logger;

import java.io.*;

/**
 * @author Aleksandr Khalitov
 */
class ResultsManager {

  private static final Logger logger = Logger.getLogger(ResultsManager.class);

  private File resultsDir;

  private FileSystem fs;

  public ResultsManager(File resultsDir, FileSystem fs) {
    this.resultsDir = resultsDir;
    this.fs = fs;
  }

  public DeliveriesResult createDeliveriesResutls(int requestId) throws AdminException{
    return new DeliveriesResult(requestId);
  }

  public MessagesResult createMessagesResutls(int requestId) throws AdminException{
    return new MessagesResult(requestId);
  }

  public void getDeliveriesResults(int requestId, Visitor<ArchiveDelivery> visitor) throws AdminException {
    File file = new File(resultsDir, Request.Type.deliveries+"."+requestId+".csv");
    if(!file.exists()) {
      logger.warn("Results don't exist: requestId="+requestId);
      return;
    }
    BufferedReader reader = null;
    try{
      reader = new BufferedReader(new InputStreamReader(fs.getInputStream(file)));
      String line;
      try {
        while((line = reader.readLine()) != null) {
          if(line.length() == 0) {
            continue;
          }
          ArchiveDelivery delivery = new ArchiveDelivery();
          RequestSerializer.deserialize(line, delivery);
          if(!visitor.visit(delivery)) {
            break;
          }
        }
      } catch (Exception e) {
        logger.error(e,e);
        throw new ArchiveException("internal_error");
      }
    }finally {
      if(reader != null) {
        try {
          reader.close();
        } catch (IOException ignored) {}
      }
    }
  }

  public void getMessagesResults(int requestId, Visitor<ArchiveMessage> visitor) throws AdminException {
    File file = new File(resultsDir, Request.Type.messages+"."+requestId+".csv");
    if(!file.exists()) {
      logger.warn("Results don't exist: requestId="+requestId);
      return;
    }
    BufferedReader reader = null;
    try{
      reader = new BufferedReader(new InputStreamReader(fs.getInputStream(file)));
      String line;
      try {
        while((line = reader.readLine()) != null) {
          if(line.length() == 0) {
            continue;
          }
          ArchiveMessage message = new ArchiveMessage();
          RequestSerializer.deserialize(line, message);
          if(!visitor.visit(message)) {
            break;
          }
        }
      } catch (Exception e) {
        e.printStackTrace();
        logger.error(e,e);
        throw new ArchiveException("internal_error");
      }
    }finally {
      if(reader != null) {
        try {
          reader.close();
        } catch (IOException ignored) {}
      }
    }
  }



  class DeliveriesResult {

    private PrintWriter w;

    private DeliveriesResult(int requestId) throws AdminException {
      w = new PrintWriter(new BufferedWriter(new OutputStreamWriter(fs.getOutputStream(
          new File(resultsDir, Request.Type.deliveries+"."+requestId+".csv"), false))));
    }

    void write(Delivery delivery, DeliveryStatistics statistics, DeliveryStatusHistory statusHistory) {
        RequestSerializer.serialize(w, delivery, statistics, statusHistory);
    }

    void close() {
      if(w != null) {
        w.close();
      }
    }
  }


  class MessagesResult {

    private PrintWriter w;

    private MessagesResult(int requestId) throws AdminException {
      w = new PrintWriter(new BufferedWriter(new OutputStreamWriter(fs.getOutputStream(
          new File(resultsDir, Request.Type.messages+"."+requestId+".csv"), false))));
    }

    void write(Delivery delivery, Message message) {
        RequestSerializer.serialize(w, delivery, message);
    }

    void close() {
      if(w != null) {
        w.close();
      }
    }
  }

}
