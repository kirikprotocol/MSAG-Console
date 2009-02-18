package ru.sibinco.smsx.engine.service.group.datasource.impl;

import com.eyeline.utils.FileUtils;
import com.eyeline.utils.IOUtils;

import java.io.*;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

/**
 * User: artem
 * Date: 19.11.2008
 */
public class DataFile {

  protected static final int ADDRESS_LEN = 23;
  protected static final int FLAG_LEN = 1;
  protected static final int VERSION_LEN = 4;

  protected int recordsNumber;
  protected RandomAccessFile f;
  protected final Lock lock = new ReentrantLock();

  protected final String header;
  protected int version;
  protected int recordLen;

  protected DataFile(File file, String header, int version, int recordLen) throws IOException {
    this.header = header;
    this.version = version;
    this.recordLen = recordLen;

    int headerLen = header.length();

    byte[] bytes = new byte[headerLen + 4];

    if (file.exists()) { // Check header, version and file length
      f = new RandomAccessFile(file, "rw");
      f.seek(0);
      f.readFully(bytes);

      String headerStr = new String(bytes, 0, headerLen);
      if (!headerStr.equals(header))
        throw new IOException("Illegal file " + file.getName() + " header: " + headerStr + ". Should be " + header);

      int ver = IOUtils.readInt(bytes, headerLen);
      if (ver != version)
        throw new IOException("Illegal file " + file.getName() + " version: " + ver + ". Should be " + version);

      long dataLen = f.length() - headerLen - VERSION_LEN;
      long mod = dataLen % (recordLen + FLAG_LEN);
      if (mod != 0) {
        long len = f.length() - mod;
        f.close();
        repairFile(file, len);
        f = new RandomAccessFile(file, "rw");
      }

      recordsNumber = (int) (dataLen/(recordLen + FLAG_LEN));

    } else { // Insert header and version

      if (!file.getParentFile().exists() && !file.getParentFile().mkdirs())
        throw new IOException("Can't create directories for " + file.getAbsolutePath());

      IOUtils.writeString(header, bytes, 0, headerLen);
      IOUtils.writeInt(version, bytes, headerLen);

      FileOutputStream writer = null;
      try {
        writer = new FileOutputStream(file);
        writer.write(bytes);
      } finally {
        if (writer != null)
          writer.close();
      }

      f = new RandomAccessFile(file, "rw");
    }
  }

  private static void repairFile(File file, long len) throws IOException {
    System.out.println("File " + file.getName() + " is crushed and will be repaired.");

    SimpleDateFormat sdf = new SimpleDateFormat("yyyyMMddHHmmss");
    File fileCopy = new File(file.getParentFile(), file.getName() + ".crushed." + sdf.format(new Date()));
    System.out.println("Crushed file " + file.getName() + " will be copied to " + fileCopy.getName());
    try {
      FileUtils.copyFile(file, fileCopy);
    } catch (IOException e) {
      System.out.println("File copy error");
      e.printStackTrace();
      throw e;
    }

    System.out.println("Crushed file " + file.getName() + " will be truncated to " + len + " bytes.");

    try {
      FileUtils.truncateFile(file, len);
    } catch (IOException e) {
      System.out.println("File truncation error");
      e.printStackTrace();
      throw e;
    }

    System.out.println("File " + file.getName() + " was successfully repaired.");
  }

  protected void list(RecordVisitor visitor) throws IOException {
    byte bytes[] = new byte[recordLen];
    try {
      lock.lock();

      f.seek(header.length() + VERSION_LEN);

      long pointer;
      while(true) {
        pointer = f.getFilePointer();

        int flag = f.read();
        if (flag == 1) {
          try {
            f.readFully(bytes);
          } catch (EOFException e) {
            break;
          }
          visitor.record(bytes, pointer);
        } else
          if (f.skipBytes(recordLen) != recordLen)
            break;
      }
    } finally {
      lock.unlock();
    }
  }

  protected long append(byte[] bytes) throws IOException {
    if (bytes.length != recordLen)
      throw new IllegalArgumentException("Invalid bytes len " + bytes.length + ", should be " + recordLen);

    try {
      lock.lock();
      long offset = f.length();
      f.seek(offset);
      f.write(1);
      f.write(bytes);
      recordsNumber++;
      return offset;
    } finally {
      lock.unlock();
    }
  }

  protected void write(long offset, byte[] bytes) throws IOException {
    try {
      lock.lock();
      f.seek(offset + FLAG_LEN);
      f.write(bytes);
      recordsNumber++;
    } finally {
      lock.unlock();
    }
  }

  protected void remove(long offset) throws IOException {
    try {
      lock.lock();
      f.seek(offset);
      f.write(0);
      recordsNumber--;
    } finally {
      lock.unlock();
    }
  }

  private static String getAddressString(int ton, int npi, String address) {
    StringBuilder sb = new StringBuilder(ADDRESS_LEN);
    if(ton == 1 && npi == 1)
      return sb.append('+').append(address).toString();
    else if( ton == 0 && npi == 1 )
      return sb.append(address).toString();
    else
      return sb.append('.').append(ton).append('.').append(npi).append('.').append(address).toString();
  }

  protected static String readMsisdn(byte[] bytes, int offset) {
    int ton = bytes[offset];
    int npi = bytes[offset + 1];
    String address = IOUtils.readString(bytes, offset + 2, ADDRESS_LEN);
    return getAddressString(ton, npi, address);
  }

  protected static void writeMsisdn(String msisdn, byte[] bytes, int offset) {
    int ton, npi;
    String address;
    if (msisdn.charAt(0) == '+') {
      ton = 1;
      npi = 1;
      address = msisdn.substring(1);
    } else {
      ton = 0;
      npi = 1;
      address = msisdn;
    }

    bytes[offset] = (byte)ton;
    bytes[offset + 1] = (byte)npi;
    IOUtils.writeString(address, bytes, offset + 2, ADDRESS_LEN);
  }

  public int getRecordsNumber() {
    return recordsNumber;
  }

  public void close() {
    try {
      lock.lock();
      f.close();
    } catch (IOException e) {
    } finally {
      lock.unlock();
    }
  }

  public interface RecordVisitor {
    public void record(byte[] bytes, long pointer);
  }
}
