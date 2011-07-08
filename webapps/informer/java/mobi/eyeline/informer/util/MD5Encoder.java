package mobi.eyeline.informer.util;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;

/**
 * User: artem
 * Date: 07.07.11
 */
public class MD5Encoder {

  static byte[] createChecksum(InputStream is) throws IOException {
    try {
      MessageDigest complete = MessageDigest.getInstance("MD5");

      byte[] buffer = new byte[1024];
      int numRead;
      do {
        numRead = is.read(buffer);
        if (numRead > 0) {
          complete.update(buffer, 0, numRead);
        }
      } while (numRead != -1);

      return complete.digest();
    } catch (NoSuchAlgorithmException ignored) {
      return null;
    }
  }

  public static String calcMD5Checksum(InputStream is) throws IOException {
    byte[] b = createChecksum(is);
    String result = "";
    for (byte aB : b)
      result += Integer.toString((aB & 0xff) + 0x100, 16).substring(1);
    return result;
  }

}
