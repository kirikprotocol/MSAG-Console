package ru.sibinco.sponsored.stats.backend.datasource;

import org.apache.log4j.Category;
import ru.sibinco.sponsored.stats.backend.StatisticsException;

import java.io.*;
import java.nio.channels.FileChannel;
import java.nio.channels.WritableByteChannel;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;

/**
 * @author Aleksandr Khalitov
 */
class StatsFileImpl implements StatsFile{

   private static final Category log = Category.getInstance(StatsFileImpl.class);

  private static final int AGGR_BUFFER_SIZE = 200000;
  private static final int EOR = 255;
  private static final int MAX_RECORD_SIZE = 15;

  private OutputStream os;
  private final File file;

  StatsFileImpl(File file) throws IOException {
    this.file = file;
    this.os = null;
    open(true);
  }

  private static void verifyAndRepairFile(File file) throws IOException {
    if (!file.exists())
      return;

    RandomAccessFile f = null;
    try {
      f = new RandomAccessFile(file, "rw");

      long endPos = f.length() - 1;
      if (endPos < 0)
        return;

      f.seek(endPos);
      if (f.read() != EOR) {
        log.warn("File " + file.getAbsolutePath() + " is corrupted and will be repaired.");

        // Search last EOR
        log.warn("Search for last EOR label");
        long pos = -1;
        int step = 1;
        do {
          long startPos = f.length() - step * MAX_RECORD_SIZE;

          if (startPos < 0) {
            pos = 0;
          } else {
            f.seek(startPos);
            int b;
            while ((b = f.read()) != -1) {
              if (b == EOR)
                pos = f.getFilePointer();
            }
          }
          step++;
        } while (pos == -1);

        log.warn("Last EOR label pos is " + pos + ". Truncate file.");

        // Truncate file after last EOR
        FileChannel fc = null;
        try {
          fc = f.getChannel();
          fc.truncate(pos);
        } finally {
          if (fc != null)
            fc.close();
        }

        log.warn("File was successfully repaired.");
      }

    } catch (EOFException ignored) {
    } catch (FileNotFoundException ignored) {
    } catch (IOException e) {
      log.error("Verify and repair error",e);
      throw new IOException(e.getMessage());
    } finally {
      try {
        if (f != null)
          f.close();
      } catch (IOException e) {
        log.error(e,e);
      }
    }
  }

  private void open(boolean check) throws IOException {
    if (os != null) // Already opened
      return;

    if (check)
      verifyAndRepairFile(file);

    os = new BufferedOutputStream(new FileOutputStream(file, true));
  }

  public String getName() {
    return file.getAbsolutePath();
  }

  private static DeliveryStat readDeliveryStat(InputStream is) throws IOException {
    final DeliveryStat s = new DeliveryStat();
    s.setSubscriberAddress(readString8(is));
    s.setAdvertiserId(readInt(is));
    s.setDelivered(readByte(is));
    s.setSended(readByte(is));
    readByte(is); // EOR
    return s;
  }

  private static void writeDeliveryStat(DeliveryStat stat, OutputStream os) throws IOException {
    writeString8(stat.getSubscriberAddress(), os);
    writeInt(stat.getAdvertiserId(), os);
    os.write(stat.getDelivered());
    os.write(stat.getSended());
    os.write(EOR);
  }

  private static void skipDeliveryStat(InputStream is) throws IOException {
    readString8(is);// msisdn
    readInt(is);    // advertiser id
    readByte(is);   // delivered
    readByte(is);   // sended
    readByte(is);   // EOR
  }

  private static void writeInt(int value, OutputStream os) throws IOException {
    os.write((value >> 24) & 0xFF);
    os.write((value >> 16) & 0xFF);
    os.write((value >> 8) & 0xFF);
    os.write(value & 0xFF);
  }


  private static int readInt(InputStream is) throws IOException {
    int ch1 = readByte(is);
    int ch2 = readByte(is);
    int ch3 = readByte(is);
    int ch4 = readByte(is);
    return (((ch1 << 24) & 0xFF000000) | ((ch2 << 16) & 0x00FF0000) | ((ch3 << 8) & 0x0000FF00) | (ch4 & 0x000000FF));
  }

  private static String readString8(InputStream is) throws IOException {
    int len = readByte(is);
    byte bytes[] = new byte[len];
    readFully(is, bytes);
    return new String(bytes);
  }

  private static void writeString8(String str, OutputStream os) throws IOException {
    byte[] bytes = str.getBytes();
    writeByte(bytes.length, os);
    os.write(bytes);
  }

  private static void writeByte(int value, OutputStream os) throws IOException {
    os.write(value & 0xFF);
  }


  private static int readByte(InputStream is) throws IOException {
    int b = is.read();
    if (b == -1)
      throw new EOFException();
    return b;
  }


  private static void readFully(InputStream is, byte b[]) throws IOException {
    readFully(is, b, b.length);
  }

  private static void readFully(InputStream is, byte b[], int len) throws IOException {
    int n = 0;
    do {
      int count = is.read(b, n, len - n);
      if (count < 0)
        throw new EOFException();
      n += count;
    } while (n < len);
  }


  public void transferTo(DeliveryStatsQuery query, OutputStream target) throws StatisticsException {
    InputStream is = null;
    try {
      os.flush();

      is = new BufferedInputStream(new FileInputStream(file));

      while(true) {
        DeliveryStat s = readDeliveryStat(is);
        if (query.isAllowed(s))
          writeDeliveryStat(s, target);
      }

    } catch (EOFException ignored) {
    } catch (IOException e) {
      throw new StatisticsException(e.getMessage(), e);
    } finally {
      try {
        if (is != null)
          is.close();
      } catch (IOException e) {
        log.error(e,e);
      }
    }
  }

  public void transferTo(WritableByteChannel target) throws StatisticsException {
    FileInputStream is = null;
    try {
      os.flush();

      is = new FileInputStream(file);

      FileChannel fc = null;
      try {
        fc = is.getChannel();

        fc.transferTo(0, fc.size(), target);
      } catch (IOException e) {
        throw new StatisticsException(e.getMessage(), e);
      } finally {
        if (fc != null)
          fc.close();
      }

    } catch (IOException e) {
      throw new StatisticsException(e.getMessage(), e);
    } finally {
      try {
        if (is != null)
          is.close();
      } catch (IOException e) {
        log.error(e,e);
      }
    }
  }

  public void addStat(DeliveryStat stat) throws StatisticsException {
    try {
      writeDeliveryStat(stat, os);
    } catch (IOException e) {
      throw new StatisticsException("Can't add stats to file " + file, e);
    }
  }

  public ArrayList getRecords(int start, int count) throws StatisticsException {
    ArrayList result = new ArrayList(count);
    InputStream is = null;
    try {
      is = new BufferedInputStream(new FileInputStream(file));

      for (int i=0; i<start; i++)
        skipDeliveryStat(is);

      for (int i=start; i < start + count; i++)
        result.add(readDeliveryStat(is));

    } catch (EOFException ignored) {
    } catch (IOException e) {
      log.error("Can't get stats from file " + file, e);
      throw new StatisticsException("Can't get stats from file " + file, e);
    } finally {
      try {
        if (is != null)
          is.close();
      } catch (IOException e) {
        log.error(e,e);
      }
    }

    return result;
  }

  private static void fillBuffer(InputStream curFileReader, Map buffer, DeliveryStatsQuery query) throws IOException {
    // We read file until buffer size < AGGR_BUFFER_SIZE or EOF reached. Every delivery stat we put into buffer.
    while (buffer.size() < AGGR_BUFFER_SIZE) {
      DeliveryStat stat = readDeliveryStat(curFileReader);
      if (query == null || query.isAllowed(stat)) {
        DeliveryStat oldStat = (DeliveryStat)buffer.get(stat);
        if (oldStat == null)
          buffer.put(stat, stat);
        else {
          oldStat.setDelivered(oldStat.getDelivered() + stat.getDelivered());
          oldStat.setSended(oldStat.getSended() + stat.getSended());
        }
      }
    }
  }

  private static void transferOther(InputStream curFileReader, Map buffer, DeliveryStatsQuery query, OutputStream tmpFileWriter) throws IOException {
    // Read up to the end of current file
    // Update counters in buffer and write other records to tmp file
    while(true) {
      DeliveryStat stat = readDeliveryStat(curFileReader);
      if (query == null || query.isAllowed(stat)) {
        DeliveryStat oldStat = (DeliveryStat)buffer.get(stat);
        if (oldStat == null)
          writeDeliveryStat(stat, tmpFileWriter);
        else {
          oldStat.setDelivered(oldStat.getDelivered() + stat.getDelivered());
          oldStat.setSended(oldStat.getSended() + stat.getSended());
        }
      }
    }
  }

  public void compress(DeliveryStatsQuery query) throws StatisticsException {

    // Close output stream if it was opened and save status to variable
    final boolean opened = os != null;
    if (os != null)
      close();

    // Create buffer
    final Map buffer = new HashMap(AGGR_BUFFER_SIZE);

    final File compressedFile = new File(file.getAbsolutePath() + ".compressed");
    OutputStream compressedFileWriter = null;

    try {
      compressedFileWriter = new BufferedOutputStream(new FileOutputStream(compressedFile));

      File tmpFile = null, curFile = file;

      int iterNo = 0;
      do {

        buffer.clear();

        InputStream curFileReader = null;
        OutputStream tmpFileWriter = null;

        try {
          curFileReader = new BufferedInputStream(new FileInputStream(curFile));

          // Fill buffer
          fillBuffer(curFileReader, buffer, iterNo == 0 ? query : null);

          // We fill buffer but EOF was not reached
          // This mean that there are other delivery stats. We copy them into tmp file
          tmpFile = new File(file.getAbsolutePath() + ".tmp" + iterNo);

          tmpFileWriter = new BufferedOutputStream(new FileOutputStream(tmpFile));

          // Transfer other
          transferOther(curFileReader, buffer, iterNo == 0 ? query : null, tmpFileWriter);

        } catch (EOFException e) {
          // EOF is expected
        } finally {
          try {
            if (curFileReader != null)
              curFileReader.close();
          } catch (IOException e) {
            log.error(e,e);
          }
          try {
            if (tmpFileWriter != null)
              tmpFileWriter.close();
          } catch (IOException e) {
            log.error(e,e);
          }
        }

        // Remove current (temp) file if it not equals to initial file
        if (curFile != file) {
          if (!curFile.delete())
            log.warn("Can't delete tmp file " + curFile.getAbsolutePath());
        }

        if (buffer.isEmpty()) // No records in file. Break
          break;

        // At this moment we have buffer and temp file with records outside buffer.

        // We flush aggregated records into .compressed file
        Iterator i = buffer.values().iterator();
        while(i.hasNext()){
          DeliveryStat e = (DeliveryStat)i.next();
          writeDeliveryStat(e,compressedFileWriter);
        }

        curFile = tmpFile;
        tmpFile = null;

        // Increment counter
        iterNo++;
      } while (curFile != null);

    } catch (IOException e) {
      log.error(e,e);
      throw new StatisticsException("Compression file " + file + " error", e);
    } finally {
      try {
        if (compressedFileWriter != null)
          compressedFileWriter.close();
      } catch (IOException e) {
        log.error(e,e);
      }
    }

    File bakFile = new File(file.getAbsolutePath() + ".bak");
    if (file.renameTo(bakFile)) { // Rename stats file to .bak

      if (compressedFile.renameTo(file)) { // Rename compressed file to stats file

        if (!bakFile.delete()) // Delete .bak file
          log.warn("Can't delete file " + bakFile);

      } else { // Can't rename compressed file to stats file. Try to rollback rename stats file to .bak
        log.warn("Can't rename compressed file " + compressedFile + " to " + file);

        if (!bakFile.renameTo(file))  // ERROR! We have no stats file!
          log.error("Can't rename file " + bakFile + " to " + file);
      }

    } else {
      log.warn("Can't rename uncompressed file " + file + " to " + bakFile);
    }

    // If file was opened before compression we open it
    if (opened) {
      try {
        open(false);
      } catch (IOException e) {
        log.error(e,e);
        throw new StatisticsException("Can't open output stream to file " + file, e);
      }
    }
  }

  public void close() throws StatisticsException {
    if (os != null) {
      try {
        os.close();
      } catch (IOException e) {
        throw new StatisticsException("Can't close stats file " + file, e);
      }
      os = null;
    }
  }

}
