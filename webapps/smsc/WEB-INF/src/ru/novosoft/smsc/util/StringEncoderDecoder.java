package ru.novosoft.smsc.util;

/*
 * Created by igork
 * Date: Apr 11, 2002
 * Time: 5:26:52 PM
 */


public class StringEncoderDecoder
{
  protected static final char ESCAPE_CHAR = '/';

  public static String encodeDot(String str)
  {
    return str.replace('.', ESCAPE_CHAR);
  }

  public static String decodeDot(String str)
  {
    return str.replace(ESCAPE_CHAR, '.');
  }

  public static String encode(String str)
  {
    if (str == null) return "";
    StringBuffer result = new StringBuffer(str.length());
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

  public static String encodeHEX(String str)
  {
    if (str == null) return "";
    byte b[] = str.getBytes();
    int c = 0;
    StringBuffer sb = new StringBuffer(b.length * 2);
    for (int i = 0; i < str.length(); i++) {
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

  public static String decodeHEX(String str)
  {
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
    return new String(b);
  }
}
