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
    for (byte aB : b) {
      c = (((int) aB) >> 4) & 0xf;
      if (c < 10)
        sb.append((char) ('0' + c));
      else
        sb.append((char) ('A' + (c - 10)));

      c = ((int) aB) & 0xf;
      if (c < 10)
        sb.append((char) ('0' + c));
      else
        sb.append((char) ('A' + (c - 10)));
    }
    return sb.toString();
  }

  public static String getHexString(byte val[]) {
    StringBuilder sb = new StringBuilder();
    for (byte aVal : val) {
      sb.append(' ');
      int k = (((int) aVal) >> 4) & 0xf;
      sb.append((char) (k < 10 ? k + 48 : k + 55));
      k = ((int) aVal) & 0xf;
      sb.append((char) (k < 10 ? k + 48 : k + 55));
    }
    return sb.toString().toUpperCase();
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

  private static final String specialSaveChars = "=: \t\r\n\f#!";

  private static final char[] hexDigit = {
      '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'
  };

  private static char toHex(int nibble) {
    return hexDigit[(nibble & 0xF)];
  }

  public static String stringToUnicode(String theString) {
    boolean escapeSpace = false;
    int len = theString.length();
    StringBuilder outBuffer = new StringBuilder(len << 1);

    for(int x=0; x<len; x++) {
      char aChar = theString.charAt(x);
      switch(aChar) {
        case ' ':
          if (x == 0 || escapeSpace)
            outBuffer.append('\\');

          outBuffer.append(' ');
          break;
        case '\\':outBuffer.append('\\'); outBuffer.append('\\');
          break;
        case '\t':outBuffer.append('\\'); outBuffer.append('t');
          break;
        case '\n':outBuffer.append('\\'); outBuffer.append('n');
          break;
        case '\r':outBuffer.append('\\'); outBuffer.append('r');
          break;
        case '\f':outBuffer.append('\\'); outBuffer.append('f');
          break;
        default:
          if ((aChar < 0x0020) || (aChar > 0x007e)) {
            outBuffer.append('\\');
            outBuffer.append('u');
            outBuffer.append(toHex((aChar >> 12) & 0xF));
            outBuffer.append(toHex((aChar >>  8) & 0xF));
            outBuffer.append(toHex((aChar >>  4) & 0xF));
            outBuffer.append(toHex( aChar        & 0xF));
          } else {
            if (specialSaveChars.indexOf(aChar) != -1)
              outBuffer.append('\\');
            outBuffer.append(aChar);
          }
      }
    }
    return outBuffer.toString();
  }

  public static String unicodeToString(String theString) {
    char aChar;
    int len = theString.length();
    StringBuilder outBuffer = new StringBuilder(len);

    for (int x=0; x<len; ) {
      aChar = theString.charAt(x++);
      if (aChar == '\\') {
        aChar = theString.charAt(x++);
        if (aChar == 'u') {
          // Read the xxxx
          int value=0;
          for (int i=0; i<4; i++) {
            aChar = theString.charAt(x++);
            switch (aChar) {
              case '0': case '1': case '2': case '3': case '4':
              case '5': case '6': case '7': case '8': case '9':
                value = (value << 4) + aChar - '0';
                break;
              case 'a': case 'b': case 'c':
              case 'd': case 'e': case 'f':
                value = (value << 4) + 10 + aChar - 'a';
                break;
              case 'A': case 'B': case 'C':
              case 'D': case 'E': case 'F':
                value = (value << 4) + 10 + aChar - 'A';
                break;
              default:
                throw new IllegalArgumentException(
                    "Malformed \\uxxxx encoding.");
            }
          }
          outBuffer.append((char)value);
        } else {
          if (aChar == 't') aChar = '\t';
          else if (aChar == 'r') aChar = '\r';
          else if (aChar == 'n') aChar = '\n';
          else if (aChar == 'f') aChar = '\f';
          outBuffer.append(aChar);
        }
      } else
        outBuffer.append(aChar);
    }
    return outBuffer.toString();
  }
}
