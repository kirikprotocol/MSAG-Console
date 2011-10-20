package ru.sibinco.smsx.stats;

import java.io.*;
import java.util.Date;

/**
 * User: artem
 * Date: 20.10.11
 */
public class ArcFileReader {

  private ArcFileReader() {
  }

  private static void readFile(File f) {
    InputStream input = null;
    boolean haveArc=false;

    try {
      input = new BufferedInputStream(new FileInputStream(f));

      System.out.println(IOUtils.readString(input, 8));
      long version = IOUtils.readUInt16(input);
      if (version > 0x010000)
        haveArc = true;

      while (true) {

        int msgSize1 = (int) IOUtils.readUInt32(input);
        byte[] message = new byte[msgSize1];
        IOUtils.readFully(input, message);

        ByteArrayInputStream is = new ByteArrayInputStream(message);

        long msgId = IOUtils.readInt64(is);  // 8 bytes
        int state = IOUtils.readUInt8(is);
        Date submitTime = new Date(IOUtils.readUInt32(is) * 1000);
        Date validTime = new Date(IOUtils.readUInt32(is) * 1000);
        Date lastTime = new Date(IOUtils.readUInt32(is) * 1000);
        Date nextTime = new Date(IOUtils.readUInt32(is) * 1000);
        long attempts = IOUtils.readUInt32(is);
        long lastResult = IOUtils.readUInt32(is);
        String sourceAddress = IOUtils.readString8(is);
        String destinationAddress = IOUtils.readString8(is);
        String dealiasedDestinationAddress = IOUtils.readString8(is);
        int messageReference = IOUtils.readUInt16(is);
        String svcType = IOUtils.readString8(is);
        int deliveryReport = IOUtils.readUInt8(is);
        int billingRecord = IOUtils.readUInt8(is);
        String sourceMscAddress = IOUtils.readString8(is);
        String sourceImsi = IOUtils.readString8(is);
        long sourceSme = IOUtils.readUInt32(is);
        String destinationMscAddress = IOUtils.readString8(is);
        String destinationImsi = IOUtils.readString8(is);
        long destinationSme = IOUtils.readUInt32(is);
        String route = IOUtils.readString8(is);
        long serviceId = IOUtils.readUInt32(is);
        long priority = IOUtils.readUInt32(is);
        String sourceSmeId = IOUtils.readString8(is);

        System.out.println(msgId + "   " + state + "   " + sourceSmeId + "    " + state);

        int msgSize2 = (int) IOUtils.readUInt32(input);
        if (msgSize1 != msgSize2)
          throw new IOException("Protocol error sz1=" + msgSize1 + " sz2=" + msgSize2);

      }

    } catch (EOFException ignored) {
    } catch (Exception e) {
      e.printStackTrace();
    } finally {
      if (input != null)
        try {
          input.close();
        } catch (IOException ignored) {
        }
    }
  }


  public static void main(String[] args) {
    readFile(new File("00.bin"));
  }

}
