package com.eyeline.sponsored.ds.distribution.advert.impl.file.deliveries;

import com.eyeline.sponsored.ds.DataSourceException;
import com.eyeline.sponsored.ds.distribution.advert.Delivery;
import static com.eyeline.sponsored.ds.distribution.advert.impl.file.deliveries.RWUtils.*;
import org.apache.log4j.Category;

import java.io.*;
import java.util.*;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

/**
 * User: artem
 * Date: 28.03.2008
 */

public class DeliveriesFileImpl implements DeliveriesFile {

  private static final Category log = Category.getInstance(DeliveriesFileImpl.class);

  // Record fields
  private static final int MSISDN_LEN = 21;
  private static final int START_DATE_LEN = 4;
  private static final int END_DATE_LEN = 4;
  private static final int SEND_DATE_LEN = 4;
  private static final int SENDED_LEN = 1;
  private static final int TOTAL_LEN = 1;
  private static final int EOR_LEN = 1;
  private static final byte EOR = (byte)0xFF;

  private static final int RECORD_LEN = MSISDN_LEN + START_DATE_LEN + END_DATE_LEN + SEND_DATE_LEN + SENDED_LEN + TOTAL_LEN + EOR_LEN;

  // Volume fields
  private static final int TX_NAME_LEN = 20;
  private static final int TX_TZ_LEN = 40;
  private static final int TX_VOLUME_LEN =1;
  private static final int TX_START_LEN =8;
  private static final int TX_END_LEN =8;

  private static final int TX_DESCRIPTION_LEN = TX_NAME_LEN + TX_TZ_LEN + TX_VOLUME_LEN + TX_START_LEN + TX_END_LEN + EOR_LEN;

  private static final int TX_NUMBER = 100;

  private final long date;
  private final FileDeliveriesDataSource ds;

  private final TransactionManager transactionManager;

  private RandomAccessFile f;
  private final Lock rwlock = new ReentrantLock();

  public DeliveriesFileImpl(FileDeliveriesDataSource ds, File file, long date) throws DeliveriesFileException {
    this.ds = ds;
    this.date = date;

    try {
      rwlock.lock();

      this.f = new RandomAccessFile(file, "rw");

    } catch (FileNotFoundException e) {
      throw new DeliveriesFileException("Can't open file", e);
    } finally {
      rwlock.unlock();
    }

    this.transactionManager = new TransactionManager();
  }

  public DeliveriesFileTransaction createTransaction(String distrName, int volume, TimeZone tz, int size) throws DeliveriesFileException {
    return transactionManager.createTx(distrName, volume, tz, size);
  }

  public void updateDelivery(DeliveryImpl d) throws DeliveriesFileException {
    if (d.getId() < 0)
      throw new DeliveriesFileException("Can't update delivery with id < 0");

    final byte[] bytes = new byte[RECORD_LEN];
    writeDelivery(d, bytes, 0);

    try {
      rwlock.lock();

      f.seek(d.getId());
      f.write(bytes);

    } catch (Exception e) {
      throw new DeliveriesFileException("Can't update delivery", e);
    } finally {
      rwlock.unlock();
    }
  }

  /**
   *
   * @param startDate
   * @param endDate
   * @throws DeliveriesFileException
   */
  public void readDeliveries(final Date startDate, final Date endDate, Collection<Delivery> result) throws DeliveriesFileException {
    readDeliveries(new DeliveriesQuery() {
      public boolean add(DeliveryImpl d) {
        double a1 = (double)(d.getTotal() - 1) * (startDate.getTime() - d.getStartDate().getTime()) / (d.getEndDate().getTime() - d.getStartDate().getTime());
        double a2 = (double)(d.getTotal() - 1) * (endDate.getTime() - d.getStartDate().getTime()) / (d.getEndDate().getTime() - d.getStartDate().getTime());
        return ((Math.ceil(a1)>=0 || a2 > 0) && Math.ceil(a1) < d.getTotal() && Math.ceil(a1) < a2);
      }
    }, result);
  }

  /**
   *
   * @param date
   * @param limit
   * @throws DeliveriesFileException
   */
  public void readDeliveries(final Date date, final int limit, Collection<Delivery> result) throws DeliveriesFileException {
    readDeliveries(new DeliveriesQuery() {
      int total = 0;
      public boolean add(DeliveryImpl d) {
        return (total++ < limit && d.getSended() < d.getTotal() && d.getSendDate().getTime() < date.getTime());
      }
    }, result);
  }

  /**
   *
   * @param date
   * @param tz
   * @param distrName
   * @return
   * @throws DeliveriesFileException
   */
  public int getDeliveriesCount(final Date date, TimeZone tz, String distrName) throws DeliveriesFileException {
    int count = 0;
    try {
      final DeliveriesQuery ds = new DeliveriesQuery() {
        public boolean add(DeliveryImpl d) {
          return d.getEndDate().after(date);
        }
      };

      final List<Delivery> lst = new LinkedList<Delivery>();
      for (FileTransaction tx : transactionManager.commitedTransactions()) {
        if (tx.distrName.equals(distrName) && tx.tz.equals(tz)) {
          tx.getDeliveries(ds, lst);
          count += lst.size();
          lst.clear();
        }
      }

    } catch (EOFException e) {
    } catch (IOException e) {
      throw new DeliveriesFileException("Can't read deliveries", e);
    }
    return count;
  }

  private void readDeliveries(DeliveriesQuery st, Collection<Delivery> result) throws DeliveriesFileException {
    try {
      for (FileTransaction tx : transactionManager.commitedTransactions())
        tx.getDeliveries(st, result);

    } catch (EOFException e) {
    } catch (IOException e) {
      throw new DeliveriesFileException("Can't read deliveries", e);
    }
  }


  public void close() throws DeliveriesFileException {
    transactionManager.close();

    try {
      rwlock.lock();
      f.close();
    } catch (IOException e) {
      throw new DeliveriesFileException("Can't close file", e);
    } finally {
      rwlock.unlock();
    }
  }


  private void writeDelivery(DeliveryImpl impl, byte[] bytes, int start) {
    // Record len
    int pos = start;
    // Subscriber address
    writeString(impl.getSubscriberAddress(), bytes, pos, MSISDN_LEN);
    pos += MSISDN_LEN + 1;
    // Start date
    writeInt((int)(impl.getStartDate().getTime() - date), bytes, pos);
    // End date
    writeInt((int)(impl.getEndDate().getTime() - date), bytes, pos + 4);
    // Sended
    bytes[pos + 8] = (byte)impl.getSended();
    // Send date
    writeInt((int)(impl.getSendDate().getTime() - date), bytes, pos + 9);
    // EOR
    bytes[pos + 13] = (byte)EOR;
  }





  private interface DeliveriesQuery {
    public boolean add(DeliveryImpl d);
  }






  private class TransactionManager {

    private final List<FileTransaction> commitedTransactions = new ArrayList<FileTransaction>(TX_NUMBER);
    private final List<FileTransaction> newTransactions = new ArrayList<FileTransaction>(TX_NUMBER);

    private final Lock lock = new ReentrantLock();

    private int actualTxsNumber;



    public TransactionManager() throws DeliveriesFileException {
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
        throw new DeliveriesFileException("Can't read volumes", e);
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

        final FileTransaction s = readTx(bytes, pos);
        if (s != null)
          commitedTransactions.add(s);
      }
    }

    private FileTransaction readTx(byte[] bytes, int startPos) throws DeliveriesFileException {
      int pos = startPos;
      String name = readString(bytes, pos, TX_NAME_LEN);
      pos += TX_NAME_LEN;
      String tz = readString(bytes, pos, TX_TZ_LEN);
      pos += TX_TZ_LEN;
      byte volume = bytes[pos];
      long start = readLong(bytes, pos + 1);
      long end = readLong(bytes, pos + 9);
      byte eor = bytes[pos + 17];
      if (eor != EOR) {
        log.error("Section descriptor is crushed: name=" + name + "; vol=" + volume + "; tz=" + tz);
        return null;
      } else
        return new FileTransaction(name, TimeZone.getTimeZone(tz), volume, start, end, true);
    }

    private void writeTx(FileTransaction s, byte[] bytes, int startPos) {
      int pos = startPos;
      writeString(s.distrName, bytes, pos, TX_NAME_LEN);
      pos+=TX_NAME_LEN;
      writeString(s.tz.getID(), bytes, pos, TX_TZ_LEN);
      pos+=TX_TZ_LEN;
      bytes[pos] = (byte)s.volume;
      writeLong(s.sectionStartPos, bytes, pos + 1);
      writeLong(s.sectionEndPos, bytes, pos + 9);
      bytes[pos + 17] = (byte)EOR;
    }

    public void commitTx(FileTransaction tx) throws DeliveriesFileException {
      byte[] bytes = new byte[TX_DESCRIPTION_LEN];
      writeTx(tx, bytes, 0);

      try {
        rwlock.lock();

        f.seek(actualTxsNumber * TX_DESCRIPTION_LEN);
        f.write(bytes);

      } catch (IOException e) {
        throw new DeliveriesFileException("Can't save transaction", e);
      } finally {
        rwlock.unlock();
      }

      try {
        lock.lock();
        actualTxsNumber++;

        commitedTransactions.add(tx);
        newTransactions.remove(tx);
      } finally {
        lock.unlock();
      }
    }

    public FileTransaction createTx(String distrName, int volume, TimeZone tz, int size) throws DeliveriesFileException {
      try {
        lock.lock();

        if (actualTxsNumber >= TX_NUMBER)
          throw new DeliveriesFileException("Max sections number reached.");

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
          throw new DeliveriesFileException("Can't create section", e);
        } finally {
          rwlock.unlock();
        }

        FileTransaction tx = new FileTransaction(distrName, tz, volume, startPos, endPos, false);


        newTransactions.add(tx);
        return tx;
      } finally {
        lock.unlock();
      }
    }

    public List<FileTransaction> commitedTransactions() {
      try {
        lock.lock();

        return new ArrayList<FileTransaction>(commitedTransactions);
      } finally {
        lock.unlock();
      }
    }

    public void close() {
      try {
        lock.lock();

        for (FileTransaction tx : newTransactions)
          tx.close();

      } finally {
        lock.unlock();
      }
    }
  }





















  private class FileTransaction implements DeliveriesFileTransaction {

    private final String distrName;
    private final TimeZone tz;
    private final int volume;

    private final long sectionStartPos;
    private long sectionCurPos;
    private final long sectionEndPos;

    private boolean closed;

    private byte[] buffer;
    private long bufferStartPos;
    private int bufferPos;
    private int bufferSize;
    private DeliveryImpl curDelivery;

    public FileTransaction(String distrName, TimeZone timezone, int volume, long startPos, long endPos, boolean closed) throws DeliveriesFileException {
      this.distrName = distrName;
      this.volume = volume;
      this.tz = timezone;
      this.closed = closed;

      this.curDelivery = null;
      this.sectionStartPos = startPos;
      this.sectionEndPos = endPos;

      this.buffer = new byte[65536];
      bufferStartPos = startPos;
      bufferPos = 0;
      bufferSize = 0;

      try {
        if (closed)
          validateAndRepair();
        else
          sectionCurPos = sectionStartPos;
      } catch (IOException e) {
        throw new DeliveriesFileException("Can't init section", e);
      }
    }

    private void validateAndRepair() throws IOException {
      // Check last byte
      long endPos = sectionEndPos -1;
      if (endPos < sectionStartPos)
        return;

      try {
        rwlock.lock();

        f.seek(endPos);
        byte eor = (byte)f.read();

        sectionCurPos = sectionEndPos;

        if (eor != EOR) {
          log.debug("Section is not full: distrName=" + distrName + "; tz=" + tz.getID() + "; vol=" + volume);

          long pos = -1;
          int step = 1;
          do {
            long startPos = sectionEndPos - step * RECORD_LEN;

            if (startPos < sectionStartPos) {
              pos = sectionStartPos;
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

          sectionCurPos = pos;
        }

      } finally {
        rwlock.unlock();
      }
    }

    private DeliveryImpl readDelivery() throws IOException {
      final DeliveryImpl d = new DeliveryImpl(ds);
      d.setId(getFilePointer());

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
      pos += MSISDN_LEN + 1;
      // Start date
      d.setStartDate(new Date(readInt(bytes, pos) + date));
      // End date
      d.setEndDate(new Date(readInt(bytes, pos + 4) + date));
      // Sended
      d.setSended(bytes[pos+8]);
      // Send date
      d.setSendDate(new Date(readInt(bytes, pos + 9) + date));
      d.setDistributionName(distrName);
      d.setTimezone(tz);
      d.setTotal(volume);
      return d;
    }

    public void saveDelivery(DeliveryImpl delivery) throws DataSourceException {
      if (closed)
        throw new DataSourceException("Tx already closed");

      byte[] bytes = new byte[RECORD_LEN];
      writeDelivery(delivery, bytes, 0);

      try {
        rwlock.lock();

        f.seek(delivery.getId() < 0 ? sectionCurPos : delivery.getId());
        f.write(bytes);
        sectionCurPos += RECORD_LEN;

      } catch (Exception e) {
        throw new DataSourceException(e);
      } finally {
        rwlock.unlock();
      }
    }

    public void getDeliveries(DeliveriesQuery st, Collection<Delivery> result) throws IOException {
      if (getFilePointer() >= sectionEndPos) {
        bufferStartPos = sectionStartPos;
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
          curDelivery = readDelivery();
          if (getFilePointer() >= sectionEndPos) {
            bufferStartPos = sectionStartPos;
            bufferPos=0;
            bufferSize = 0;
          }
        }

      } while (f1==f2 && getFilePointer() != startPos);
    }

    public void commit() throws DataSourceException {
      try {
        transactionManager.commitTx(this);
        closed = true;
      } catch (DeliveriesFileException e) {
        throw new DataSourceException(e);
      }
    }

    public void rollback() throws DataSourceException {
    }

    public void close() {
      closed = true;
    }

    private long getFilePointer() {
      return bufferStartPos + bufferPos;
    }

    private byte read() throws IOException {
      if (bufferPos >= bufferSize)
        fillBuffer();
      return buffer[bufferPos++];
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

      int actualSize = (int)Math.min(sectionEndPos - bufferStartPos, buffer.length);
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
  }

}
