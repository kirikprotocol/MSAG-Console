package com.eyeline.sponsored.ds.distribution.advert.impl.file.deliveries;

import com.eyeline.sponsored.ds.distribution.advert.Delivery;
import org.apache.log4j.Category;

import java.io.*;
import java.util.*;

/**
 * User: artem
 * Date: 28.03.2008
 */

public class DeliveriesFile {

  private static final Category log = Category.getInstance(DeliveriesFile.class);

  private static final int START_DATE_LEN = 4;
  private static final int END_DATE_LEN = 4;
  private static final int SEND_DATE_LEN = 4;
  private static final int SENDED_LEN = 1;
  private static final int TOTAL_LEN = 1;
  private static final int EOR_LEN = 1;
  private static final byte EOR = (byte)0xFF;

  private static final int RECORD_FIELDS_LEN = START_DATE_LEN + END_DATE_LEN + SEND_DATE_LEN + SENDED_LEN + TOTAL_LEN + EOR_LEN;

  private static final int SECTION_NAME_LEN = 20;
  private static final int SECTION_TZ_LEN = 40;
  private static final int SECTIONS_NUMBER = 100;

  private static final int SECTION_DESCRIPTION_LEN = SECTION_NAME_LEN + SECTION_TZ_LEN + 1/*Volume*/ + 8/*start*/  + 1/*EOR*/;

  private final File file;
  private final long date;
  private final FileDeliveriesDataSource ds;

  private final SectionsMap sections;
  private Section lastSection;

  private RandomAccessFile f;

  public DeliveriesFile(FileDeliveriesDataSource ds, File file, long date) throws DeliveriesFileException {
    this.ds = ds;
    this.file = file;
    this.date = date;
    this.sections = new SectionsMap(SECTIONS_NUMBER);
    this.lastSection = null;
    open();
  }

  private void open() throws DeliveriesFileException {
    try {
      if (f == null) {
        boolean exists = file.exists();
        this.f = new RandomAccessFile(file, "rw");
        if (exists) {
          readSections();
        } else
          writeSections();
      }

    } catch (FileNotFoundException e) {
      throw new DeliveriesFileException("Can't open file", e);
    } catch (IOException e) {
      throw new DeliveriesFileException("Can't read sections", e);
    }
  }

  private static void writeString(String s, byte[] bytes, int offset, int len) {
    int i;
    for (i=0; i<s.length(); i++)
      bytes[offset + i] = (byte)s.charAt(i);
    for (int j=i; j<len; j++)
      bytes[offset+j]=(byte)' ';
  }

  private static String readString(byte[] bytes, int offset, int len) {
    StringBuilder sb = new StringBuilder(len);
    char ch;
    for (int i=offset; i<offset + len; i++) {
      ch = (char)bytes[i];
      if (ch != ' ')
        sb.append(ch);
      else
        break;
    }
    return sb.toString();
  }

  private static void writeInt(int value, byte[] bytes, int offset) {
    bytes[offset]     = (byte) ((value >> 24) & 0xFF);
    bytes[offset + 1] = (byte) ((value >> 16) & 0xFF);
    bytes[offset + 2] = (byte) ((value >> 8) & 0xFF);
    bytes[offset + 3] = (byte) ((value) & 0xFF);
  }

  private static int readInt(byte[] bytes, int offset) {
    int ch1 = bytes[offset];
    int ch2 = bytes[offset+1];
    int ch3 = bytes[offset+2];
    int ch4 = bytes[offset+3];
    return (((ch1 << 24) & 0xFF000000) | ((ch2 << 16) & 0x00FF0000) | ((ch3 << 8) & 0x0000FF00) | (ch4 & 0x000000FF));
  }

  private static void writeLong(long value, byte[] bytes, int offset) {
    bytes[offset]     = (byte) ((value >> 56) & 0xFF);
    bytes[offset + 1] = (byte) ((value >> 48) & 0xFF);
    bytes[offset + 2] = (byte) ((value >> 40) & 0xFF);
    bytes[offset + 3] = (byte) ((value >> 32) & 0xFF);
    bytes[offset + 4] = (byte) ((value >> 24) & 0xFF);
    bytes[offset + 5] = (byte) ((value >> 16) & 0xFF);
    bytes[offset + 6] = (byte) ((value >> 8) & 0xFF);
    bytes[offset + 7] = (byte) ((value) & 0xFF);
  }

  private static long readLong(byte[] bytes, int offset) {
    long ch1 = bytes[offset];
    long ch2 = bytes[offset+1];
    long ch3 = bytes[offset+2];
    long ch4 = bytes[offset+3];
    long ch5 = bytes[offset+4];
    long ch6 = bytes[offset+5];
    long ch7 = bytes[offset+6];
    long ch8 = bytes[offset+7];
    return (
            ((ch1 << 56) & 0xFF00000000000000L) |
            ((ch2 << 48) & 0x00FF000000000000L) |
            ((ch3 << 40) & 0x0000FF0000000000L) |
            ((ch4 << 32) & 0x000000FF00000000L) |
            ((ch5 << 24) & 0x00000000FF000000L) |
            ((ch6 << 16) & 0x0000000000FF0000L) |
            ((ch7 << 8)  & 0x000000000000FF00L) |
            (ch8         & 0x00000000000000FFL)
           );
  }

  private void readSections() throws IOException {
    f.seek(0);
    byte[] bytes = new byte[SECTIONS_NUMBER * SECTION_DESCRIPTION_LEN];
    f.readFully(bytes);

    Section prevSection = null;
    for (int i = 0; i < SECTIONS_NUMBER; i++) {

      int pos = i * SECTION_DESCRIPTION_LEN;

      // Read section descriptor
      String name = readString(bytes, pos, SECTION_NAME_LEN);
      pos += SECTION_NAME_LEN;
      String tz = readString(bytes, pos, SECTION_TZ_LEN);
      pos += SECTION_TZ_LEN;
      byte volume = bytes[pos];
      long start = readLong(bytes, pos + 1);
      byte eor = bytes[pos + 9];

      if (name.length() != 0) {
        if (eor != EOR) {
          log.error("Section descriptor is crushed: name=" + name + "; vol=" + volume + "; tz=" + tz);
          // clear section descriptor
          f.seek(pos);
          for (long j = pos; j < pos + SECTION_DESCRIPTION_LEN; j++)
            f.write(0);
          continue;
        }

        final Section s = new Section(name, TimeZone.getTimeZone(tz), volume, start);
        if (prevSection != null) {
          prevSection.sectionEndPos = start;
          prevSection.extendable = false;
        }
        sections.addSection(s);
        prevSection = s;
      }
    }

    if (prevSection != null) {
      prevSection.sectionEndPos = f.length();
      lastSection = prevSection;
    }

    // Validate and repair last section
    if (lastSection != null)
      lastSection.validateAndRepair();
  }

  private void writeSections() throws IOException {
    byte[] bytes = new byte[SECTIONS_NUMBER * SECTION_DESCRIPTION_LEN];
    Arrays.fill(bytes, (byte)' ');
    f.seek(0);
    f.write(bytes);

    int pos=0;
    for (Section s : sections.sections()) {
      writeString(s.distrName, bytes, pos, SECTION_NAME_LEN);
      pos+=SECTION_NAME_LEN;
      writeString(s.tz.getID(), bytes, pos, SECTION_TZ_LEN);
      pos+=SECTION_TZ_LEN;
      bytes[pos] = (byte)s.volume;
      writeLong(s.sectionStartPos, bytes, pos + 1);
      bytes[pos + 9] = (byte)EOR;
      pos += 10;
    }
    f.seek(0);
    f.write(bytes);
  }

  /**
   *
   * @param delivery
   * @throws DeliveriesFileException
   */
  public void saveDelivery(DeliveryImpl delivery) throws DeliveriesFileException {
    try {
      // Lookup section
      Section s = sections.getSection(delivery.getDistributionName(), delivery.getTimezone(), delivery.getTotal());

      if (s == null) { // If section does not exists create it
        final long flen = f.length();

        s = new Section(delivery.getDistributionName(), delivery.getTimezone(), delivery.getTotal(), flen);

        if (lastSection != null)
          lastSection.close();
        lastSection = s;

        sections.addSection(s);

        writeSections();
      }
      s.saveDelivery(delivery);
    } catch (IOException e) {
      throw new DeliveriesFileException("Save error", e);
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

  public int getDeliveriesCount(final Date date, TimeZone tz, String distrName) throws DeliveriesFileException {
    int count = 0;
    try {
      final DeliveriesQuery ds = new DeliveriesQuery() {
        public boolean add(DeliveryImpl d) {
          return d.getEndDate().after(date);
        }
      };

      for (Section s : sections.sections()) {
        List<Delivery> lst = new LinkedList<Delivery>();
        if (s.distrName.equals(distrName) && s.tz.equals(tz)) {
          s.getDeliveries(ds, lst);
          count += lst.size();
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
      for (Section s : sections.sections())
        s.getDeliveries(st, result);

    } catch (EOFException e) {
    } catch (IOException e) {
      throw new DeliveriesFileException("Can't read deliveries", e);

    }
  }


  public void close() throws DeliveriesFileException {
    try {
      if (f != null)
        f.close();
    } catch (IOException e) {
      throw new DeliveriesFileException("Can't close file", e);
    }
  }




  private interface DeliveriesQuery {
    public boolean add(DeliveryImpl d);
  }






  private static class SectionsMap {
    private final HashMap<String, Section> sections;

    public SectionsMap(int size) {
      sections = new HashMap<String, Section>(size);
    }

    public synchronized void addSection(Section s) {
      sections.put(s.distrName + '_' + s.tz.getID() + '_' + s.volume, s);
    }

    public synchronized Section getSection(String name, TimeZone tz, int volume) {
      return sections.get(name + '_' + tz.getID() + '_' + volume);
    }

    public synchronized Collection<Section> sections() {
      return new LinkedList<Section>(sections.values());
    }
  }






  private class Section {

    private final String distrName;
    private final TimeZone tz;
    private final int volume;

    private final long sectionStartPos;
    private long sectionEndPos;

    private boolean extendable;

    private byte[] buffer;
    private long bufferStartPos;
    private int bufferPos;
    private int bufferSize;
    private DeliveryImpl curDelivery;

    public Section(String distrName, TimeZone timezone, int volume, long startPos) {
      this.distrName = distrName;
      this.volume = volume;
      this.tz = timezone;

      this.curDelivery = null;
      this.sectionStartPos = startPos;
      this.sectionEndPos = startPos;

      this.extendable = true;

      this.buffer = new byte[65536];
      bufferStartPos = startPos;
      bufferPos = 0;
      bufferSize = 0;
    }

    private void validateAndRepair() throws IOException {
      // Check last byte
      long endPos = sectionEndPos -1;
      if (endPos < sectionStartPos)
        return;

      f.seek(endPos);
      byte eor = (byte)f.read();
      if (eor != EOR) {
        log.warn("Section is corrupted and will be repaired: distrName=" + distrName + "; tz=" + tz.getID() + "; vol=" + volume);

        long pos = -1;
        int step = 1;
        do {
          long startPos = sectionEndPos - step * 10;

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

        sectionEndPos = pos;
      }
    }

    private void writeDelivery(DeliveryImpl impl) throws IOException {
      final byte[] bytes = new byte[RECORD_FIELDS_LEN + 1 + impl.getSubscriberAddress().length()];

      // Record len
      bytes[0] = (byte)(bytes.length - 1);
      int pos = 1;
      // Subscriber addr len
      bytes[pos] = (byte)impl.getSubscriberAddress().length();
      // Subscriber address
      writeString(impl.getSubscriberAddress(), bytes, pos + 1, impl.getSubscriberAddress().length());
      pos += impl.getSubscriberAddress().length() + 1;
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

      f.write(bytes);
    }

    private DeliveryImpl readDelivery() throws IOException {
      final DeliveryImpl d = new DeliveryImpl(ds);
      d.setId(getFilePointer());

      final int recordLen = read();
      if (recordLen == -1)
        throw new EOFException();

      final byte[] bytes = new byte[recordLen];
      final int l = read(bytes);
      if (l != bytes.length)
        throw new EOFException();

      // Check EOR
      if (bytes[bytes.length - 1] != EOR)
        throw new IOException("Record is crushed");

      int pos = 0;
      // Subscriber addr len
      int strlen = bytes[pos];
      // Subscriber address
      d.setSubscriberAddress(readString(bytes, pos+1, strlen));
      pos += strlen + 1;
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

    public void saveDelivery(DeliveryImpl delivery) throws DeliveriesFileException, IOException {
      if (delivery.getId() >= 0) {
        f.seek(delivery.getId());
      } else {
        if (!extendable)
          throw new DeliveriesFileException("Section is closed");
        f.seek(sectionEndPos);
      }
      writeDelivery(delivery);
      sectionEndPos = f.length();
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

    public void close() {
      extendable = false;
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

      f.seek(bufferStartPos);

      int actualSize = (int)Math.min(sectionEndPos - bufferStartPos, buffer.length);
      if (actualSize == 0)
        throw new EOFException();

      f.readFully(buffer, 0, actualSize);

      bufferPos = 0;
      bufferSize = actualSize;
    }
  }

}
