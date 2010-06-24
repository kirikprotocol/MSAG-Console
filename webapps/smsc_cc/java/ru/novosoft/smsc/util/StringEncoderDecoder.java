package ru.novosoft.smsc.util;

public class StringEncoderDecoder {
  
  protected static final char ESCAPE_CHAR = '/';

  public static String encodeDot(String str) {
    return str.replace('.', ESCAPE_CHAR);
  }

  public static String decodeDot(String str) {
    return str.replace(ESCAPE_CHAR, '.');
  }

  public static String encode(String str) {
    if (str == null) return "";
    StringBuilder result = new StringBuilder(str.length());
    for (int i = 0; i < str.length(); i++) {
      switch (str.charAt(i)) {
        case '<': // &lt;
          result.append("&lt;");
          break;
        case '>': //&gt;
          result.append("&gt;");
          break;
        case '&': //&amp;
          result.append("&amp;");
          break;
        case '"': //&quot;
          result.append("&quot;");
          break;
        default:
          result.append(str.charAt(i));
      }
    }
    return result.toString();
  }

  public static String encodeHTML(String str) {
    if (str == null) return "";
    StringBuilder result = new StringBuilder(str.length());
    for (int i = 0; i < str.length(); i++) {
      switch (str.charAt(i)) {
        case '<': // &lt;
          result.append("&lt;");
          break;
        case '>': //&gt;
          result.append("&gt;");
          break;
        case '&': //&amp;
          result.append("&amp;");
          break;
        case '"': //&quot;
          result.append("&quot;");
          break;
        case '\n':
          result.append("<br/>");
          break;
        default:
          result.append(str.charAt(i));
      }
    }
    return result.toString();
  }

  private static byte[] stringToBytes(String str) {
    byte[] bytes = new byte[str.length() * 2];
    int c;
    for (int i = 0; i < str.length(); i++) {
      c = str.charAt(i);
      bytes[i * 2] = (byte) (c >> 8);
      bytes[i * 2 + 1] = (byte) c;
    }
    return bytes;
  }

  private static String bytesToString(byte[] bytes) {
    StringBuilder buffer = new StringBuilder();
    for (int i = 0; i < bytes.length; i += 2)
      buffer.append((char) ((bytes[i] << 8) | (bytes[i + 1])));
    return buffer.toString();
  }

  public static String encodeHEX(String str) {
    if (str == null) return "";
    byte b[] = stringToBytes(str);
    int c = 0;
    StringBuilder sb = new StringBuilder(b.length * 2);
    for (int i = 0; i < b.length; i++) {
      c = (((int) b[i]) >> 4) & 0xf;
      if (c < 10)
        sb.append((char) ('0' + c));
      else
        sb.append((char) ('A' + (c - 10)));

      c = ((int) b[i]) & 0xf;
      if (c < 10)
        sb.append((char) ('0' + c));
      else
        sb.append((char) ('A' + (c - 10)));
    }
    return sb.toString();
  }

  public static String decodeHEX(String str) {
    if (str.length() % 2 != 0) throw new RuntimeException("HEX encoded string should contain odd number of chracters");
    byte b[] = new byte[str.length() / 2];
    char c;
    int ci;
    for (int i = 0; i < str.length();) {
      c = str.charAt(i);
      if (c >= '0' && c <= '9')
        ci = ((int) (c - '0')) << 4;
      else if (c >= 'A' && c <= 'F')
        ci = ((int) (c - 'A' + 10)) << 4;
      else
        throw new RuntimeException("Invalid char '" + c + "' detected in HEX encoded string");

      c = str.charAt(i + 1);
      if (c >= '0' && c <= '9')
        ci |= (int) (c - '0');
      else if (c >= 'A' && c <= 'F')
        ci |= (int) (c - 'A' + 10);
      else
        throw new RuntimeException("Invalid char '" + c + "' detected in HEX encoded string");

      b[i / 2] = (byte) ci;
      i += 2;
    }
    return bytesToString(b);
  }
}
