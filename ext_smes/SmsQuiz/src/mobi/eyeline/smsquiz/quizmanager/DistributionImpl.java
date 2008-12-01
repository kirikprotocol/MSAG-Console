package mobi.eyeline.smsquiz.quizmanager;

import mobi.eyeline.smsquiz.distribution.Distribution;
import mobi.eyeline.smsquiz.storage.ResultSet;
import mobi.eyeline.smsquiz.storage.StorageException;
import mobi.eyeline.smsquiz.subscription.SubManagerException;
import mobi.eyeline.smsquiz.subscription.SubscriptionManager;
import org.apache.log4j.Logger;

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.io.File;
import java.util.concurrent.ScheduledExecutorService;

public class DistributionImpl extends Distribution {

  private static final Logger logger = Logger.getLogger(DistributionImpl.class);

  private String abFile;

  private SubscriptionManager subscriptionManager;

  private ScheduledExecutorService executor;

  public DistributionImpl(String abFile, SubscriptionManager subscriptionManager) throws QuizException{
    if(abFile==null) {
      logger.error("Some arguments are null");
      throw  new IllegalArgumentException("Some arguments are null");
    }
    if(!new File(abFile).exists()) {
      logger.error("Abonents file doesn't exist: "+abFile);
      throw  new QuizException("Abonents file doesn't exist: "+abFile);

    }
    this.abFile = abFile;
    this.subscriptionManager = subscriptionManager;
  }


  public ResultSet abonents() {
    return new AbonentsResultSet(new File(abFile));
  }

  private class AbonentsResultSet implements ResultSet{

    private File file;

    private BufferedReader reader = null;

    private String currentAbonent = null;

    AbonentsResultSet(File file) {
      if(file==null) {
        logger.error("Some arguments are null");
        throw new IllegalArgumentException("Some arguments are null");
      }
      this.file = file;
    }

    public boolean next() throws StorageException {
      try {
        if(reader == null) {
          reader = new BufferedReader(new FileReader(file));
        }
        String line;
        while ((line = reader.readLine()) != null) {
          if (subscribed(line))
            return true;
        }
        reader.close();
        return false;
      } catch (Exception e) {
        logger.error("Error during getting line from file "+abFile, e);
        if (reader != null) {
          try {
            reader.close();
          } catch (IOException e1) {
            logger.error("Can't close reader", e1);
          }
        }
        throw new StorageException("Error during getting line from file", e);
      }
    }

    private boolean subscribed(String line) throws SubManagerException{
      String msisdn = line.trim();
      if (subscriptionManager.subscribed(msisdn)) {
        currentAbonent = msisdn;
        return true;
      }
      return false;
    }

    public Object get() throws StorageException {
      return currentAbonent;
    }

    public void close() {
      if(reader!=null) {
        try {
          reader.close();
        } catch (IOException e) {}
      }
    }

  }

}

