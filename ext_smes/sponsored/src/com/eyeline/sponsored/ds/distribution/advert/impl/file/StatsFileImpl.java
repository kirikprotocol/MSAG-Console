package com.eyeline.sponsored.ds.distribution.advert.impl.file;

import com.eyeline.sponsored.ds.distribution.advert.DeliveryStat;

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

public class StatsFileImpl implements StatsFile {

  private static final int AGGR_BUFFER_SIZE = 200000;
  private static final int IO_BUFFER_SIZE = 4096;
  private static final int EOR = 255;
  private static final int MAX_RECORD_SIZE = 15;

  private OutputStream os;
  private File file;

  public StatsFileImpl(File file) throws IOException {
    this.file = file;
    open(true);
  }

  private static void verifyAndRepairFile(File file)  {
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
        System.out.println("File " + file.getAbsolutePath() + " is corrupted and will be repaired.");

        // Search last EOR
        long pos = -1;
        for (int step = 1; pos == -1; step++) {
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
        }

        // Truncate file after last EOR
        FileChannel fc = null;
        try {
          fc = f.getChannel();
          fc.truncate(pos);
        } finally {
          if (fc != null)
            fc.close();
        }
      }

    } catch (EOFException e) {
    } catch (FileNotFoundException e) {
    } catch (IOException e) {
      System.out.println(e.getMessage());
    } finally {
      try {
        if (f != null)
          f.close();
      } catch (IOException e) {
      }
    }
  }

  private void open(boolean check) throws IOException {
    if (os != null) // Already opened
      return;

    if (check)
      verifyAndRepairFile(file);

    this.os = new BufferedOutputStream(new FileOutputStream(file, true), IO_BUFFER_SIZE);
  }

  public String getName() {
    return file.getAbsolutePath();
  }

  private static DeliveryStatImpl readDeliveryStat(InputStream is) throws IOException {
    final DeliveryStatImpl s = new DeliveryStatImpl();
    s.setSubscriberAddress(IOUtils.readString(is));
    s.setDelivered(IOUtils.readByte(is));
    IOUtils.readByte(is); // EOR
    return s;
  }

  private static void writeDeliveryStat(DeliveryStat stat, OutputStream os) throws IOException {
    IOUtils.writeString(stat.getSubscriberAddress(), os);
    os.write(stat.getDelivered());
    os.write(EOR);
  }

  private static void skipDeliveryStat(InputStream is) throws IOException {
    IOUtils.skipString(is); // msisdn
    IOUtils.readByte(is);   // count
    IOUtils.readByte(is);   // EOR
  }

  public void transferTo(WritableByteChannel target) throws IOException {
    FileInputStream is = null;
    try {
      os.flush();

      is = new FileInputStream(file);

      FileChannel fc = null;
      try {
        fc = is.getChannel();

        fc.transferTo(0, fc.size(), target);
      } finally {
        if (fc != null)
          fc.close();
      }

    } catch (EOFException e) {
    } finally {
      try {
        if (is != null)
          is.close();
      } catch (IOException e) {
      }
    }
  }

  public void addStat(DeliveryStat stat) throws IOException {
    writeDeliveryStat(stat, os);
  }

  public ArrayList<DeliveryStatImpl> getRecords(int start, int count) throws IOException {
    ArrayList<DeliveryStatImpl> result = new ArrayList<DeliveryStatImpl>(count);
    InputStream is = null;
    try {
      is = new BufferedInputStream(new FileInputStream(file), IO_BUFFER_SIZE);

      for (int i=0; i<start; i++)
        skipDeliveryStat(is);

      for (int i=start; i < start + count; i++)
        result.add(readDeliveryStat(is));

    } catch (EOFException e) {
    } finally {
      try {
        if (is != null)
          is.close();
      } catch (IOException e) {
      }
    }

    return result;
  }

  public void compress() throws IOException {
    final boolean opened = os != null;

    if (os != null)
      close();

    final Map<String, DeliveryStat> buffer = new HashMap<String, DeliveryStat>(AGGR_BUFFER_SIZE);

    final File compressedFile = new File(file.getAbsolutePath() + ".compressed");
    OutputStream compressedFileWriter = null;

    try {

      compressedFileWriter = new BufferedOutputStream(new FileOutputStream(compressedFile), IO_BUFFER_SIZE);

      File tmpFile, curFile = file;

      for (int iterNo = 0; curFile != null; iterNo++) {

        buffer.clear();

        InputStream curFileReader = null;
        OutputStream tmpFileWriter = null;

        tmpFile = null;
        try {
          curFileReader = new BufferedInputStream(new FileInputStream(curFile), IO_BUFFER_SIZE);

          // Fill buffer
          while (buffer.size() < AGGR_BUFFER_SIZE) {
            DeliveryStat stat = readDeliveryStat(curFileReader);
            DeliveryStat oldStat = buffer.get(stat.getSubscriberAddress());
            if (oldStat == null)
              buffer.put(stat.getSubscriberAddress(), stat);
            else
              oldStat.setDelivered(oldStat.getDelivered() + stat.getDelivered());
          }

          tmpFile = new File(file.getAbsolutePath() + ".tmp" + iterNo);

          tmpFileWriter = new BufferedOutputStream(new FileOutputStream(tmpFile), IO_BUFFER_SIZE);

          // Read up to the end of current file
          // Update counters in buffer and write other records to tmp file
          while(true) {
            DeliveryStat stat = readDeliveryStat(curFileReader);

            DeliveryStat oldStat = buffer.get(stat.getSubscriberAddress());
            if (oldStat != null)
              oldStat.setDelivered(oldStat.getDelivered() + stat.getDelivered());
            else
              writeDeliveryStat(stat, tmpFileWriter);
          }

        } catch (EOFException e) {

        } finally {
          try {
            if (curFileReader != null)
              curFileReader.close();
          } catch (IOException e) {
          }

          try {
            if (tmpFileWriter != null) {
              tmpFileWriter.flush();
              tmpFileWriter.close();
            }
          } catch (IOException e) {
          }
        }

        if (buffer.isEmpty())
          break;

        // At this moment we have buffer and temp file with records outside buffer

        for (DeliveryStat e : buffer.values())
          writeDeliveryStat(e,compressedFileWriter);

        // Remove current file if it not equals to initial file
        if (curFile != file)
          curFile.delete();

        curFile = tmpFile;
      }

      // Remove current file if it not equals to initial file
      if (curFile != null && curFile != file)
        curFile.delete();

    } finally {
      try {
        if (compressedFileWriter != null) {
          compressedFileWriter.flush();
          compressedFileWriter.close();
        }
      } catch (IOException e) {
      }
    }

    File bakFile = new File(file.getAbsolutePath() + ".bak");
    file.renameTo(bakFile);
    compressedFile.renameTo(file);

    if (opened)
      open(false);

    bakFile.delete();
  }

  public void close() throws IOException {
    if (os != null) {
      os.flush();
      os.close();
      os = null;
    }
  }

  public static void main(String[] args) {
    try {
      long time = System.currentTimeMillis();
      final StatsFileImpl impl = new StatsFileImpl(new File("stats.test"));
      System.out.println("Open time = " + (System.currentTimeMillis() - time));

      time = System.currentTimeMillis();
      for (int i=0; i<20; i++) {
        for (int j=100000; j<300002; j++) {
          final DeliveryStat s = new DeliveryStatImpl();
          s.setSubscriberAddress("+79139" + j);
          s.setDelivered(1);
          impl.addStat(s);
        }
      }
      System.out.println("Fill time = " + (System.currentTimeMillis() - time));

      time = System.currentTimeMillis();
      impl.compress();
      System.out.println("Compress time = " + (System.currentTimeMillis() - time));

    } catch (IOException e) {
      e.printStackTrace();
    }
  }
}
