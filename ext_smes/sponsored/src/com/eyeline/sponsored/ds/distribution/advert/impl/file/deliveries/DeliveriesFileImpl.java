package com.eyeline.sponsored.ds.distribution.advert.impl.file.deliveries;

import com.eyeline.sponsored.ds.DataSourceException;
import com.eyeline.sponsored.ds.distribution.advert.Delivery;
import static com.eyeline.utils.IOUtils.*;
import org.apache.log4j.Category;

import java.io.*;
import java.util.*;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

/**
 * User: artem
 * Date: 28.03.2008
 */

class DeliveriesFileImpl implements DeliveriesFile {

  private static final Category log = Category.getInstance(DeliveriesFileImpl.class);

  // Record fields
  private static final int MSISDN_LEN = 21;
  private static final int START_DATE_LEN = 8;
  private static final int END_DATE_LEN = 8;
  private static final int SEND_DATE_LEN = 8;
  private static final int SENDED_LEN = 1;
  private static final int EOR_LEN = 1;
  private static final byte EOR = (byte)0xFF;

  private static final int RECORD_LEN = MSISDN_LEN + START_DATE_LEN + END_DATE_LEN + SEND_DATE_LEN + SENDED_LEN + EOR_LEN;

  // Volume fields
  private static final int TX_NAME_LEN = 20;
  private static final int TX_TZ_LEN = 40;
  private static final int TX_VOLUME_LEN =1;
  private static final int TX_START_LEN =8;
  private static final int TX_END_LEN =8;
  private static final int TX_START_DATE_LEN =8;
  private static final int TX_END_DATE_LEN =8;

  private static final int TX_DESCRIPTION_LEN = TX_NAME_LEN + TX_TZ_LEN + TX_VOLUME_LEN + TX_START_LEN + TX_END_LEN + TX_START_DATE_LEN + TX_END_DATE_LEN + EOR_LEN;

  private static final int TX_NUMBER = 100;

  private final TransactionManager transactionManager;

  private RandomAccessFile f;
  private final Lock rwlock = new ReentrantLock();

  public DeliveriesFileImpl(File file) throws DataSourceException {
    try {
      rwlock.lock();

      this.f = new RandomAccessFile(file, "rw");

    } catch (FileNotFoundException e) {
      throw new DataSourceException("Can't open file", e);
    } finally {
      rwlock.unlock();
    }

    this.transactionManager = new TransactionManager();
  }

  public DeliveriesFileTransaction createInsertTransaction(Date startDate, Date endDate, String distrName, int volume, TimeZone tz, int size) throws DataSourceException {
    return transactionManager.createInsertTransaction(distrName, volume, tz, size, startDate, endDate);
  }

  public void lookupDeliveries(final Date startDate, final Date endDate, Collection<Delivery> result) throws DataSourceException {
    for (CommitedTransaction tx : transactionManager.commitedTransactions())
      tx.lookupDeliveries(startDate, endDate, result);
  }

  public void lookupDeliveries(final Date date, final int limit, Collection<Delivery> result) throws DataSourceException {
    for (CommitedTransaction tx : transactionManager.commitedTransactions())
      tx.lookupDeliveries(date, limit, result);
  }

  public boolean hasDeliveries(Date endDate, TimeZone tz, String distrName) throws DataSourceException {
    for (CommitedTransaction tx : transactionManager.commitedTransactions())
      if (tx.hasDeliveries(endDate, tz, distrName))
        return true;

    return false;
  }

  public void close() throws DataSourceException {
    transactionManager.close();

    try {
      rwlock.lock();
      f.close();
    } catch (IOException e) {
      throw new DataSourceException("Can't close file", e);
    } finally {
      rwlock.unlock();
    }
  }


  private static void writeDelivery(DeliveryImpl impl, byte[] bytes, int start) {
    // Record len
    int pos = start;
    // Subscriber address
    writeString(impl.getSubscriberAddress(), bytes, pos, MSISDN_LEN);
    pos += MSISDN_LEN;
    // Start date
    writeLong(impl.getStartDate().getTime(), bytes, pos);
    // End date
    writeLong(impl.getEndDate().getTime(), bytes, pos + 8);
    // Sended
    bytes[pos + 16] = (byte)impl.getSended();
    // Send date
    writeLong(impl.getSendDate().getTime(), bytes, pos + 17);
    // EOR
    bytes[pos + 25] = (byte)EOR;
  }





  private interface DeliveriesQuery {
    public boolean add(DeliveryImpl d);
    public String printQuery();
  }






  private class TransactionManager {

    private final List<CommitedTransaction> commitedTransactions = new ArrayList<CommitedTransaction>(TX_NUMBER);
    private final List<InsertTransaction> newTransactions = new ArrayList<InsertTransaction>(TX_NUMBER);

    private final Lock lock = new ReentrantLock();

    private int actualTxsNumber;



    public TransactionManager() throws DataSourceException {
      actualTxsNumber = 0;

      final byte[] bytes = new byte[TX_NUMBER * TX_DESCRIPTION_LEN];

      // Read transactions descriptions
      try {
        rwlock.lock();

        f.seek(0);

        try {
          f.readFully(bytes);
        } catch (EOFException e) { // file is new
          // Allocate disk space
          f.seek(TX_NUMBER * TX_DESCRIPTION_LEN-1);
          f.write(0);
          return;
        }
      } catch (IOException e) {
        throw new DataSourceException("Can't read volumes", e);
      } finally {
        rwlock.unlock();
      }

      // Create transactions
      for (int i = 0; i < TX_NUMBER; i++) {
        int pos = i * TX_DESCRIPTION_LEN;

        if (bytes[pos] == 0) {
          actualTxsNumber = i;
          break;
        }

        final CommitedTransaction s = readTx(bytes, pos);
        if (s != null)
          commitedTransactions.add(s);
      }
    }

    private CommitedTransaction readTx(byte[] bytes, int startPos) throws DataSourceException {
      int pos = startPos;
      String name = readString(bytes, pos, TX_NAME_LEN);
      pos += TX_NAME_LEN;
      String tz = readString(bytes, pos, TX_TZ_LEN);
      pos += TX_TZ_LEN;
      byte volume = bytes[pos];
      long txStartPos = readLong(bytes, pos + 1);
      long txEndPos = readLong(bytes, pos + 9);
      long txStartDate = readLong(bytes, pos + 17);
      long txEndDate = readLong(bytes, pos + 25);
      byte eor = bytes[pos + 33];
      if (eor != EOR) {
        log.error("Section descriptor is crushed: name=" + name + "; vol=" + volume + "; tz=" + tz);
        return null;
      } else
        return new CommitedTransaction(name, TimeZone.getTimeZone(tz), volume, txStartPos, txEndPos, txStartDate, txEndDate);
    }

    private void writeTx(InsertTransaction s, byte[] bytes, int startPos) {
      int pos = startPos;
      writeString(s.distrName, bytes, pos, TX_NAME_LEN);
      pos+=TX_NAME_LEN;
      writeString(s.tz.getID(), bytes, pos, TX_TZ_LEN);
      pos+=TX_TZ_LEN;
      bytes[pos] = (byte)s.volume;
      writeLong(s.txStartPos, bytes, pos + 1);
      writeLong(s.txEndPos, bytes, pos + 9);
      writeLong(s.startDate, bytes, pos + 17);
      writeLong(s.endDate, bytes, pos + 25);
      bytes[pos + 33] = (byte)EOR;
    }

    public void commitTx(InsertTransaction tx) throws DataSourceException {
      byte[] bytes = new byte[TX_DESCRIPTION_LEN];
      writeTx(tx, bytes, 0);

      try {
        rwlock.lock();

        f.seek(actualTxsNumber * TX_DESCRIPTION_LEN);
        f.write(bytes);

      } catch (IOException e) {
        throw new DataSourceException("Can't save transaction", e);
      } finally {
        rwlock.unlock();
      }

      try {
        lock.lock();
        actualTxsNumber++;

        commitedTransactions.add(new CommitedTransaction(tx.distrName, tx.tz, tx.volume, tx.txStartPos, tx.txEndPos, tx.startDate, tx.endDate));
        newTransactions.remove(tx);
      } finally {
        lock.unlock();
      }
    }

    public InsertTransaction createInsertTransaction(String distrName, int volume, TimeZone tz, int size, Date startDate, Date endDate) throws DataSourceException {
      try {
        lock.lock();

        if (actualTxsNumber >= TX_NUMBER)
          throw new DataSourceException("Max sections number reached.");

        long startPos;
        long endPos;
        try {
          rwlock.lock();

          startPos = f.length();
          endPos = startPos + size * RECORD_LEN;

          // Allocate disk space
          f.seek(endPos-1);
          f.write(0);

        } catch (IOException e) {
          throw new DataSourceException("Can't create section", e);
        } finally {
          rwlock.unlock();
        }

        InsertTransaction tx = new InsertTransaction(distrName, tz, volume, startPos, endPos, startDate.getTime(), endDate.getTime());

        newTransactions.add(tx);
        return tx;
      } finally {
        lock.unlock();
      }
    }

    public List<CommitedTransaction> commitedTransactions() {
      try {
        lock.lock();

        return new ArrayList<CommitedTransaction>(commitedTransactions);
      } finally {
        lock.unlock();
      }
    }

    public void close() {
      try {
        lock.lock();

        for (InsertTransaction tx : newTransactions)
          tx.close();

      } finally {
        lock.unlock();
      }
    }
  }




















  private class InsertTransaction implements DeliveriesFileTransaction {

    private final String distrName;
    private final TimeZone tz;
    private final int volume;
    private final long startDate;
    private final long endDate;

    private final long txStartPos;
    private long txCurPos;
    private final long txEndPos;

    private boolean closed;

    public InsertTransaction(String distrName, TimeZone timezone, int volume, long startPos, long endPos, long startDate,long endDate) {
      this.distrName = distrName;
      this.volume = volume;
      this.tz = timezone;
      this.closed = false;
      this.startDate = startDate;
      this.endDate = endDate;

      this.txStartPos = startPos;
      this.txEndPos = endPos;

      txCurPos = txStartPos;
    }


    public void saveDelivery(DeliveryImpl delivery) throws DataSourceException {
      if (closed)
        throw new DataSourceException("Tx already closed");

      byte[] bytes = new byte[RECORD_LEN];
      writeDelivery(delivery, bytes, 0);

      try {
        rwlock.lock();

        f.seek(txCurPos);
        f.write(bytes);
        txCurPos += RECORD_LEN;

      } catch (Exception e) {
        throw new DataSourceException(e);
      } finally {
        rwlock.unlock();
      }
    }

    public void commit() throws DataSourceException {
      transactionManager.commitTx(this);
      closed = true;
    }

    public void rollback() throws DataSourceException {
    }

    public void close() {
      closed = true;
    }
  }






  private class CommitedTransaction implements DeliveriesFileTransaction {

    private final String distrName;
    private final TimeZone tz;
    private final int volume;
    private final long startDate;
    private final long endDate;

    private final long txStartPos;
    private final long txEndPos;

    private byte[] buffer;
    private long bufferStartPos;
    private int bufferPos;
    private int bufferSize;
    private DeliveryImpl curDelivery;

    private final Lock lock = new ReentrantLock();

    public CommitedTransaction(String distrName, TimeZone timezone, int volume, long startPos, long endPos, long startDate, long endDate) throws DataSourceException {
      this.distrName = distrName;
      this.volume = volume;
      this.tz = timezone;
      this.startDate = startDate;
      this.endDate = endDate;

      this.curDelivery = null;
      this.txStartPos = startPos;
      this.txEndPos = endPos;

      this.buffer = new byte[8192];
      bufferStartPos = startPos;
      bufferPos = 0;
      bufferSize = 0;

      try {
        validateAndRepair();
      } catch (IOException e) {
        throw new DataSourceException("Can't init section", e);
      }
    }

    private void validateAndRepair() throws IOException {
      // Check last byte
      long endPos = txEndPos -1;
      if (endPos < txStartPos)
        return;

      try {
        rwlock.lock();

        f.seek(endPos);
        byte eor = (byte)f.read();

        if (eor != EOR) {
          log.debug("Section is not full: distrName=" + distrName + "; tz=" + tz.getID() + "; vol=" + volume);

          long pos = -1;
          int step = 1;
          do {
            long startPos = txEndPos - step * RECORD_LEN;

            if (startPos < txStartPos) {
              pos = txStartPos;
            } else {
              f.seek(startPos);
              int b;
              while ((b = f.read()) != -1) {
                if ((byte)b == EOR)
                  pos = f.getFilePointer();
              }
            }
            step++;
          } while (pos == -1);

        }

      } finally {
        rwlock.unlock();
      }
    }

    private DeliveryImpl readDelivery() throws IOException {
      final DeliveryImpl d = new DeliveryImpl();

      final byte[] bytes = new byte[RECORD_LEN];
      final int l = read(bytes);
      if (l != bytes.length)
        throw new EOFException();

      // Check EOR
      if (bytes[bytes.length - 1] != EOR)
        throw new IOException("Record is crushed");

      int pos = 0;
      // Subscriber address
      d.setSubscriberAddress(readString(bytes, pos, MSISDN_LEN));
      pos += MSISDN_LEN;
      // Start date
      d.setStartDate(new Date(readLong(bytes, pos)));
      // End date
      d.setEndDate(new Date(readLong(bytes, pos + 8)));
      // Sended
      d.setSended(bytes[pos+16]);
      // Send date
      d.setSendDate(new Date(readInt(bytes, pos + 17)));
      d.setDistributionName(distrName);
      d.setTimezone(tz);
      d.setTotal(volume);
      return d;
    }

    public void saveDelivery(DeliveryImpl delivery) throws DataSourceException {
      throw new DataSourceException("Can't insert new deliveries in commited transaction");
    }

    public void lookupDeliveries(final Date startDate, final Date endDate, Collection<Delivery> result) throws DataSourceException {
      if (endDate.getTime() < this.startDate || startDate.getTime() > this.endDate)
        return;

      getDeliveries(new DeliveriesQuery() {
        public boolean add(DeliveryImpl d) {
          if (d.getTotal() == 1) {
            return startDate.getTime() < d.getStartDate().getTime() && endDate.getTime() >= d.getEndDate().getTime();
          } else {
            double totalTime = d.getEndDate().getTime() - d.getStartDate().getTime();
            double p1 = startDate.getTime() - d.getStartDate().getTime();
            double p2 = endDate.getTime() - d.getStartDate().getTime();

            double a1 = (double)(d.getTotal() - 1) * p1 / totalTime;
            double a2 = (double)(d.getTotal() - 1) * p2 / totalTime;
            return ((Math.ceil(a1)>=0 || a2 > 0) && Math.ceil(a1) < d.getTotal() && Math.ceil(a1) < a2);
          }
        }

        public String printQuery() {
          return "start=" + startDate + "; end=" + endDate;
        }
      }, result);
    }

    public void lookupDeliveries(final Date date, final int limit, Collection<Delivery> result) throws DataSourceException {
      if (date.getTime() < this.startDate || date.getTime() > this.endDate)
        return;

      getDeliveries(new DeliveriesQuery() {
        int total = 0;
        public boolean add(DeliveryImpl d) {
          return (total++ < limit && d.getSended() < d.getTotal() && d.getSendDate().getTime() < date.getTime());
        }

        public String printQuery() {
          return "date=" + date + "; limit=" + limit;
        }
      }, result);
    }

    public boolean hasDeliveries(final Date endDate, final TimeZone tz, final String distrName) throws DataSourceException {
      return (this.tz.equals(tz) && this.distrName.equals(distrName) && endDate.getTime() < this.endDate);
    }

    public void getDeliveries(DeliveriesQuery st, Collection<Delivery> result) throws DataSourceException {

      try {
        lock.lock();

        if (getFilePointer() >= txEndPos) {
          bufferStartPos = txStartPos;
          bufferPos=0;
          bufferSize = 0;
        }

        long startPos = getFilePointer();
        boolean f1=false, f2=false;
        do {
          if (curDelivery != null) {
            if (st.add(curDelivery)) {
              f1=f2=true;
              result.add(curDelivery);
              curDelivery = null;
            } else {
              f2=false;
              if (!f1)
                curDelivery = null;
            }
          }

          if (curDelivery == null) {
            try {
              curDelivery = readDelivery();
              if (getFilePointer() >= txEndPos) {
                bufferStartPos = txStartPos;
                bufferPos=0;
                bufferSize = 0;
              }
            } catch (IOException e) {
              throw new DataSourceException(e);
            }
          }

        } while (f1==f2 && getFilePointer() != startPos);
      } finally {
        lock.unlock();
      }
    }

    private long getFilePointer() {
      return bufferStartPos + bufferPos;
    }

    private int read(byte[] bytes) throws IOException {
      int totalLen = 0;
      do {
        if (bufferPos >= bufferSize)
          fillBuffer();

        final int len = Math.min(bufferSize - bufferPos, bytes.length - totalLen);
        if (len > 0) {
          System.arraycopy(buffer, bufferPos, bytes, totalLen, len);
          totalLen+=len;
          bufferPos += len;
        }

      } while(totalLen < bytes.length);

      return totalLen;
    }

    private void fillBuffer() throws IOException {
      bufferStartPos += bufferSize;

      int actualSize = (int)Math.min(txEndPos - bufferStartPos, buffer.length);
      if (actualSize == 0)
        throw new EOFException();

      try {
        rwlock.lock();

        f.seek(bufferStartPos);
        f.readFully(buffer, 0, actualSize);

      } finally {
        rwlock.unlock();
      }
      bufferPos = 0;
      bufferSize = actualSize;
    }

    public void commit() throws DataSourceException {
    }

    public void rollback() throws DataSourceException {
    }

    public void close() {
    }
  }

}
