package com.eyeline.sponsored.ds.distribution.advert.impl.file.deliverystats;

import com.eyeline.sponsored.ds.distribution.advert.DeliveryStat;
import org.apache.log4j.Category;

import java.io.*;
import java.nio.channels.FileChannel;
import java.nio.channels.WritableByteChannel;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;

/**
 * User: artem
 * Date: 24.03.2008
 */

class StatsFileImpl implements StatsFile {

  private static final Category log = Category.getInstance(StatsFileImpl.class);

  private static final int AGGR_BUFFER_SIZE = 200000;
  private static final int EOR = 255;
  private static final int MAX_RECORD_SIZE = 15;

  private OutputStream os;
  private File file;

  public StatsFileImpl(File file) throws IOException {
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

    } catch (EOFException e) {
    } catch (FileNotFoundException e) {
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

  private static DeliveryStatImpl readDeliveryStat(InputStream is) throws IOException {
    final DeliveryStatImpl s = new DeliveryStatImpl();
    s.setSubscriberAddress(IOUtils.readString(is));
    s.setAdvertiserId(IOUtils.readInt(is));
    s.setDelivered(IOUtils.readByte(is));
    s.setSended(IOUtils.readByte(is));
    IOUtils.readByte(is); // EOR
    return s;
  }

  private static void writeDeliveryStat(DeliveryStat stat, OutputStream os) throws IOException {
    IOUtils.writeString(stat.getSubscriberAddress(), os);
    IOUtils.writeInt(stat.getAdvertiserId(), os);
    os.write(stat.getDelivered());
    os.write(stat.getSended());
    os.write(EOR);
  }

  private static void skipDeliveryStat(InputStream is) throws IOException {
    IOUtils.skipString(is); // msisdn
    IOUtils.readInt(is);    // advertiser id
    IOUtils.readByte(is);   // delivered
    IOUtils.readByte(is);   // sended
    IOUtils.readByte(is);   // EOR
  }

  public void transferTo(WritableByteChannel target) throws StatsFileException {
    FileInputStream is = null;
    try {
      os.flush();

      is = new FileInputStream(file);

      FileChannel fc = null;
      try {
        fc = is.getChannel();

        fc.transferTo(0, fc.size(), target);
      } catch (IOException e) {
        throw new StatsFileException(e.getMessage(), e);
      } finally {
        if (fc != null)
          fc.close();
      }

    } catch (EOFException e) {
    } catch (IOException e) {
      throw new StatsFileException(e.getMessage(), e);
    } finally {
      try {
        if (is != null)
          is.close();
      } catch (IOException e) {
        log.error(e,e);
      }
    }
  }

  public void addStat(DeliveryStat stat) throws StatsFileException {
    try {
      writeDeliveryStat(stat, os);
    } catch (IOException e) {
      throw new StatsFileException("Can't add stats to file " + file, e);
    }
  }

  public ArrayList<DeliveryStatImpl> getRecords(int start, int count) throws StatsFileException {
    ArrayList<DeliveryStatImpl> result = new ArrayList<DeliveryStatImpl>(count);
    InputStream is = null;
    try {
      is = new BufferedInputStream(new FileInputStream(file));

      for (int i=0; i<start; i++)
        skipDeliveryStat(is);

      for (int i=start; i < start + count; i++)
        result.add(readDeliveryStat(is));

    } catch (EOFException e) {
    } catch (IOException e) {
      log.error("Can't get stats from file " + file, e);
      throw new StatsFileException("Can't get stats from file " + file, e);
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

  public void compress() throws StatsFileException {

    // Close output stream if it was opened and save status to variable
    final boolean opened = os != null;
    if (os != null)
      close();

    // Create buffer
    final Map<DeliveryStat, DeliveryStat> buffer = new HashMap<DeliveryStat, DeliveryStat>(AGGR_BUFFER_SIZE);

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
          // We read file until buffer size < AGGR_BUFFER_SIZE or EOF reached. Every delivery stat we put into buffer.
          while (buffer.size() < AGGR_BUFFER_SIZE) {
            DeliveryStat stat = readDeliveryStat(curFileReader);
            DeliveryStat oldStat = buffer.get(stat);
            if (oldStat == null)
              buffer.put(stat, stat);
            else {
              oldStat.setDelivered(oldStat.getDelivered() + stat.getDelivered());
              oldStat.setSended(oldStat.getSended() + stat.getSended());
            }
          }

          // We fill buffer but EOF was not reached
          // This mean that there are other delivery stats. We copy them into tmp file
          tmpFile = new File(file.getAbsolutePath() + ".tmp" + iterNo);

          tmpFileWriter = new BufferedOutputStream(new FileOutputStream(tmpFile));

          // Read up to the end of current file
          // Update counters in buffer and write other records to tmp file
          while(true) {
            DeliveryStat stat = readDeliveryStat(curFileReader);
            DeliveryStat oldStat = buffer.get(stat);
            if (oldStat == null)
              writeDeliveryStat(stat, tmpFileWriter);
            else {
              oldStat.setDelivered(oldStat.getDelivered() + stat.getDelivered());
              oldStat.setSended(oldStat.getSended() + stat.getSended());
            }
          }

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
        for (DeliveryStat e : buffer.values())
          writeDeliveryStat(e,compressedFileWriter);

        curFile = tmpFile;
        tmpFile = null;

        // Increment counter
        iterNo++;
      } while (curFile != null);

    } catch (IOException e) {
      log.error(e,e);
      throw new StatsFileException("Compression file " + file + " error", e);
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
        throw new StatsFileException("Can't open output stream to file " + file, e);
      }
    }
  }

  public void close() throws StatsFileException {
    if (os != null) {
      try {
        os.close();
      } catch (IOException e) {
        throw new StatsFileException("Can't close stats file " + file, e);
      }
      os = null;
    }
  }

  public static void main(String[] args) {
    try {
      long time = System.currentTimeMillis();
      final StatsFileImpl impl = new StatsFileImpl(new File("stats.test"));
      System.out.println("Open time = " + (System.currentTimeMillis() - time));

      time = System.currentTimeMillis();
      final Object o = new Object();
      for (int i=0; i<20; i++) {
        for (int j=100000; j<900002; j++) {
          final DeliveryStat s = new DeliveryStatImpl();
          s.setSubscriberAddress("+79139" + j);
//          synchronized(o) {
//            try {
//              o.wait(1);
//            } catch (InterruptedException e) {
//            }
//          }
          s.setDelivered(1);
          impl.addStat(s);
        }
      }
      System.out.println("Fill time = " + (System.currentTimeMillis() - time));

      time = System.currentTimeMillis();
//      impl.compress();
      System.out.println("Compress time = " + (System.currentTimeMillis() - time));

    } catch (IOException e) {
      e.printStackTrace();
    } catch (StatsFileException e) {
      e.printStackTrace();
    }
  }
}
