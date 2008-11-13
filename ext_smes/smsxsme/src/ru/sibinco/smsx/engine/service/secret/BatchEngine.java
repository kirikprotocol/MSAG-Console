package ru.sibinco.smsx.engine.service.secret;

import org.apache.log4j.Category;

import java.io.*;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;
import java.util.concurrent.locks.Condition;
import java.util.concurrent.TimeUnit;
import java.util.StringTokenizer;

import ru.sibinco.smsx.engine.service.secret.datasource.SecretMessage;
import ru.sibinco.smsx.engine.service.secret.datasource.SecretDataSource;
import ru.sibinco.smsx.engine.service.secret.datasource.DataSourceTransaction;
import ru.sibinco.smsx.utils.DataSourceException;

/**
 * User: artem
 * Date: 13.11.2008
 */
class BatchEngine {

  private static final Category log = Category.getInstance("SECRET");

  private final Lock lock = new ReentrantLock();
  private final Condition cond = lock.newCondition();

  private final SecretDataSource ds;
  private final Worker worker;
  private final File storeDir;

  BatchEngine(SecretDataSource ds, File storeDir, File archivesDir) {
    this.ds = ds;
    this.worker = new Worker(storeDir, archivesDir, 10000);
    this.storeDir = storeDir;
    new Thread(worker).start();
  }

  public void acceptFile(File f) {
    f.renameTo(new File(storeDir, f.getName() + ".batch"));
    cond.notify();
  }

  public void shutdown() {
    worker.shutdown();
  }

  private class Worker implements Runnable {

    private final File storeDir;
    private final File archivesDir;
    private final int checkInerval;
    private boolean started = true;

    private Worker(File storeDir, File archivesDir, int checkInterval) {
      this.storeDir = storeDir;
      this.archivesDir = archivesDir;
      this.checkInerval = checkInterval;
    }

    public void shutdown() {
      started = false;
      cond.signal();
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
        }

        tx.commit();

        if (!file.renameTo(new File(archivesDir, file.getName() + ".processed")))
          throw new DataSourceException("Can't delete file: " + file.getAbsolutePath());

      } catch (IOException e) {
        log.error("Process batch I/O error for: " + file.getAbsolutePath(), e);
        if (tx != null)
          try {
            tx.rollback();
          } catch (DataSourceException e1) {
          }
      } catch (DataSourceException e) {
        log.error("Process batch data source error for: " + file.getAbsolutePath(), e);
        if (tx != null)
          try {
            tx.rollback();
          } catch (DataSourceException e1) {
          }
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
      }
    }
  }
}
