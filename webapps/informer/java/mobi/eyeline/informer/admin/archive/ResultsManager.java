package mobi.eyeline.informer.admin.archive;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.InitException;
import mobi.eyeline.informer.admin.delivery.Delivery;
import mobi.eyeline.informer.admin.delivery.Message;
import mobi.eyeline.informer.admin.delivery.Visitor;
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

  ResultsManager(File resultsDir, FileSystem fs) throws InitException{
    this.resultsDir = resultsDir;
    try {
      if(!fs.exists(resultsDir)) {
        fs.mkdirs(resultsDir);
      }
    } catch (AdminException e) {
      throw new InitException(e);
    }
    this.fs = fs;
  }

  DeliveriesResult createDeliveriesResults(int requestId) throws AdminException{
    return new DeliveriesResult(requestId);
  }

  MessagesResult createMessagesResults(int requestId) throws AdminException{
    return new MessagesResult(requestId);
  }

  void getDeliveriesResults(int requestId, Visitor<ArchiveDelivery> visitor) throws AdminException {
    File file = buildDeliveriesFile(requestId);
    if(!fs.exists(file)) {
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

  void getMessagesResults(int requestId, Visitor<ArchiveMessage> visitor) throws AdminException {
    File file = buildMessagesFile(requestId);
    if(!fs.exists(file)) {
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

  void removeDeliveriesResult(int requestId) throws AdminException {
    fs.delete(buildDeliveriesFile(requestId));
  }
  void removeMessagesResult(int requestId) throws AdminException {
    fs.delete(buildMessagesFile(requestId));
  }

  private File buildDeliveriesFile(int requestId) {
    return new File(resultsDir, Request.Type.deliveries+"."+requestId+".csv");
  }

  private File buildMessagesFile(int requestId) {
    return new File(resultsDir, Request.Type.messages+"."+requestId+".csv");
  }

  class DeliveriesResult {

    private PrintWriter w;

    private DeliveriesResult(int requestId) throws AdminException {
      w = new PrintWriter(new BufferedWriter(new OutputStreamWriter(fs.getOutputStream(
          buildDeliveriesFile(requestId), false))));
    }

    void write(Delivery delivery) {
      RequestSerializer.serialize(w, delivery);
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
          buildMessagesFile(requestId), false))));
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
