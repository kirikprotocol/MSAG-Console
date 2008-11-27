package mobi.eyeline.smsquiz.distribution.impl;

import mobi.eyeline.smsquiz.distribution.DistributionException;
import mobi.eyeline.smsquiz.distribution.StatsDelivery;
import mobi.eyeline.smsquiz.storage.ResultSet;
import mobi.eyeline.smsquiz.storage.StorageException;
import org.apache.log4j.Logger;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Collection;
import java.util.Date;
import java.util.LinkedList;
import java.util.StringTokenizer;

class DistributionResultSet implements ResultSet {

  private static final Logger logger = Logger.getLogger(DistributionResultSet.class);

  private File currentFile;
  private StatsDelivery currentStatsDelivery;
  private LinkedList<File> files;
  private BufferedReader reader;
  private final Date startDate;
  private final Date endDate;
  private String successStatus;
  private SimpleDateFormat dateFormat;

  public DistributionResultSet(final Collection<File> files, final Date startDate, final Date endDate, final String successStatus, final String dateInFilePattern) throws DistributionException {
    if ((files == null) || (startDate == null) || (endDate == null) || (successStatus == null) || (dateInFilePattern == null)) {
      logger.error("Some arguments in constructor are null");
      throw new DistributionException("Some arguments in constructor are null", DistributionException.ErrorCode.ERROR_WRONG_REQUEST);
    }
    this.successStatus = successStatus;
    this.startDate = startDate;
    this.endDate = endDate;
    this.files = new LinkedList<File>(files);
    dateFormat = new SimpleDateFormat(dateInFilePattern);
  }

  public boolean next() throws StorageException {
    try {
      while(!files.isEmpty()) {        
        if (currentFile == null) {
          currentFile = files.remove(0);
          reader = new BufferedReader(new FileReader(currentFile));
        }

        String line;
        while ((line = reader.readLine()) != null) {
          if (parseLine(line, successStatus, startDate, endDate))
            return true;
        }

        reader.close();
        currentFile = null;
      }

      return false;
    } catch (Exception e) {
      logger.error("Error during getting line from file", e);      
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

  public Object get() throws StorageException {
    return currentStatsDelivery;
  }

  public void close() {
    if (reader != null) {
      try {
        reader.close();
      } catch (IOException e) {
        logger.error("Can't close reader", e);
      }
    }
    files.clear();
  }

  private boolean parseLine(String line, String successStatus, final Date from, final Date until) throws StorageException {
    StringTokenizer tokenizer = new StringTokenizer(line, ",");
    String status = tokenizer.nextToken();
    if (status.equals(successStatus)) {
      try {
        Date date = dateFormat.parse(tokenizer.nextToken());
        if ((date.compareTo(until) <= 0) && (date.compareTo(from) >= 0)) {
          currentStatsDelivery = new StatsDelivery(tokenizer.nextToken(), date);
          return true;
        }
      } catch (ParseException e) {
        logger.error("Unable to parse date", e);
      }
    }
    return false;
  }

}
