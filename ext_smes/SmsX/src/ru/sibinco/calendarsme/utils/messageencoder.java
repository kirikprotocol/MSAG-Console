package ru.sibinco.calendarsme.utils;

import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;

/**
 * User: artem
 * Date: Aug 3, 2006
 */

public class MessageEncoder {

  public static String encodeMD5(final String str) throws EncodeException {
    if (str == null)
      return null;

    byte[] bytes = str.getBytes();

    try {
      MessageDigest algorithm = MessageDigest.getInstance("MD5");
      algorithm.reset();
      algorithm.update(bytes);

      byte[] messageDigest = algorithm.digest();
      StringBuffer hexString = new StringBuffer();
      for (int i=0;i<messageDigest.length;i++) {
        hexString.append(Integer.toHexString(0xFF & messageDigest[i]));
      }

      return hexString.toString();
    } catch (NoSuchAlgorithmException e) {
      throw new EncodeException(e);
    }
  }

  public static class EncodeException extends Exception {
    public EncodeException(Exception e) {
      super(e);
    }
  }
}
