package ru.sibinco.smsx.engine.service.secret;

import org.apache.log4j.Category;
import ru.sibinco.smsx.engine.service.secret.datasource.DataSourceTransaction;
import ru.sibinco.smsx.engine.service.secret.datasource.SecretDataSource;
import ru.sibinco.smsx.engine.service.secret.datasource.SecretMessage;
import ru.sibinco.smsx.engine.service.secret.commands.SecretBatchCmd;
import ru.sibinco.smsx.engine.service.secret.commands.SecretGetBatchStatusCmd;
import ru.sibinco.smsx.engine.service.CommandExecutionException;
import ru.sibinco.smsx.utils.DataSourceException;

import java.io.*;
import java.util.StringTokenizer;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.locks.Condition;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

/**
 * User: artem
 * Date: 13.11.2008
 */
class BatchEngine implements SecretBatchCmd.Receiver, SecretGetBatchStatusCmd.Receiver {

  private static final Category log = Category.getInstance("SECRET");

  private final Lock lock = new ReentrantLock();
  private final Condition cond = lock.newCondition();

  private final SecretDataSource ds;
  private final Worker worker;
  private final File storeDir;
  private final File archivesDir;
  private final Lock secretBatchLock = new ReentrantLock();

  BatchEngine(SecretDataSource ds, File storeDir, File archivesDir) {
    this.ds = ds;
    this.worker = new Worker(storeDir, archivesDir, 10000);
    this.storeDir = storeDir;
    this.archivesDir = archivesDir;
    new Thread(worker).start();
  }

  public String execute(SecretBatchCmd cmd) throws CommandExecutionException {
    if (log.isInfoEnabled())
      log.info("Batch: oa=" + cmd.getSourceAddress() + "; express=" + cmd.getDestAddressSubunit());

    // Prepare file name = <time in millis>.batch
    File outputFile;
    long time = System.currentTimeMillis();
    do {
      time++;
      outputFile = new File(storeDir, String.valueOf(time) + ".batch");
    } while (outputFile.exists());

    if (log.isInfoEnabled())
      log.info("Output file: " + outputFile.getName());

    // Store batch to file
    BufferedReader is = null;
    BufferedWriter os = null;
    try {
      secretBatchLock.lock();
      is = new BufferedReader(new InputStreamReader(cmd.getDestinations()));

      os = new BufferedWriter(new FileWriter(outputFile));

      String prefix = cmd.getSourceAddress() + '|';
      String postfix = '|' + cmd.getMessage() + '|' + (cmd.getDestAddressSubunit());
      String msisdn;
      while((msisdn = is.readLine()) != null) {
        msisdn = msisdn.trim();
        if (msisdn.length() == 0)
          continue;
        StringBuilder sb = new StringBuilder(prefix.length() + msisdn.length() + postfix.length() + 2);
        sb.append(prefix).append(msisdn).append(postfix).append('\n');
        os.write(sb.toString());
      }

      os.flush();
    } catch (IOException e) {
      throw new CommandExecutionException("Can't store batch", SecretBatchCmd.ERR_SYS_ERROR);
    } finally {
      secretBatchLock.unlock();
      if (is != null)
        try {
          is.close();
        } catch (IOException e) {
        }
      if (os != null)
        try {
          os.close();
        } catch (IOException e) {
        }
    }

    try {
      lock.lock();
      cond.signal();
    } finally {
      lock.unlock();
    }

    return String.valueOf(time);
  }

  public int execute(SecretGetBatchStatusCmd cmd) throws CommandExecutionException {
    String batchId = cmd.getBatchId();
    if (worker.currentBatchId != null && worker.currentBatchId.equals(batchId))
      return worker.currentBatchCount;
    for (String file : storeDir.list()) {
      if (file.length() > batchId.length() && file.substring(0, batchId.length()+1).equals(batchId + '.'))
        return 0;
    }
    for (String file : archivesDir.list()) {
      if (file.length() > batchId.length() && file.substring(0, batchId.length()+1).equals(batchId + '.'))
        return file.endsWith("error") ? SecretGetBatchStatusCmd.BATCH_STATUS_ERROR : SecretGetBatchStatusCmd.BATCH_STATUS_PROCESSED;
    }
    return SecretGetBatchStatusCmd.BATCH_STATUS_UNKNOWN;
  }

  public void shutdown() {
    worker.shutdown();
  }

  private class Worker implements Runnable {

    private final File storeDir;
    private final File archivesDir;
    private final int checkInerval;
    private boolean started = true;
    private String currentBatchId;
    private int currentBatchCount;

    private Worker(File storeDir, File archivesDir, int checkInterval) {
      this.storeDir = storeDir;
      this.archivesDir = archivesDir;
      this.checkInerval = checkInterval;
    }

    public void shutdown() {
      started = false;
      try {
        lock.lock();
        cond.signal();
      } finally {
        lock.unlock();
      }
    }

    public void run() {
      while(started) {
        // Scan directory with batches
        File[] files = storeDir.listFiles(new FilenameFilter() {
          public boolean accept(File dir, String name) {
            return (name.length() > 5 && name.substring(name.length() - 5).equals("batch"));
          }
        });

        if (files == null || files.length == 0) {
          try {
            lock.lock();
            cond.await(checkInerval, TimeUnit.MILLISECONDS);
          } catch (InterruptedException e) {
            log.error(e,e);
          } finally {
            lock.unlock();
          }
        } else {
          // Execute every batch
          for (File f : files)
            processBatch(f);
        }
      }
    }

    private void processBatch(File file) {
      if (log.isDebugEnabled())
        log.debug("Process batch file: " + file.getName() + " ...");

      long start = System.currentTimeMillis();

      currentBatchId = file.getName().substring(0, file.getName().indexOf('.'));
      currentBatchCount = 0;

      BufferedReader r = null;
      DataSourceTransaction tx = null;
      try {
        r = new BufferedReader(new FileReader(file));
        tx = ds.createTransaction();

        int lineNo = 1;
        String line;
        while((line = r.readLine()) != null) {
          StringTokenizer st = new StringTokenizer(line, "|");
          if (st.countTokens() < 3)
            throw new IOException("Invalid line format in file: " + file.getAbsolutePath() + ", lineNo=" + lineNo);

          String oa = st.nextToken();
          String da = st.nextToken();
          String message = st.nextToken();
          int destAddressSubunit = 0;
          if (st.hasMoreTokens()) {
            try {
              destAddressSubunit = Integer.parseInt(st.nextToken().trim());
            } catch (NumberFormatException e) {
              throw new IOException("Invalid line format in file: " + file.getAbsolutePath() + ", lineNo=" + lineNo);
            }
          }

          SecretMessage m = new SecretMessage();
          m.setSourceAddress(oa);
          m.setDestinationAddress(da);
          m.setMessage(message);
          m.setDestAddressSubunit(destAddressSubunit);
          m.setConnectionName("smsx");

          ds.saveSecretMessage(m, tx);

          lineNo ++;
          currentBatchCount++;
        }

        if (!file.renameTo(new File(archivesDir, file.getName() + ".processed")))
          throw new DataSourceException("Can't delete file: " + file.getAbsolutePath());

        tx.commit();

        if (log.isDebugEnabled())
          log.debug("Batch file: " + file.getName() + " successfully processed in " + (System.currentTimeMillis() - start) + " ms.");

      } catch (Exception e) {
        log.error("Process batch error for: " + file.getAbsolutePath(), e);
        if (tx != null)
          try {
            tx.rollback();
          } catch (DataSourceException e1) {
          }
        file.renameTo(new File(archivesDir, file.getName() + ".error"));
      } finally {
        if (r != null)
          try {
            r.close();
          } catch (IOException e) {
          }
        if (tx != null)
          try {
            tx.close();
          } catch (DataSourceException e) {
          }
        currentBatchId = null;
      }
    }
  }
}
