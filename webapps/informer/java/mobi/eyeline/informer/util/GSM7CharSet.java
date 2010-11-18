package mobi.eyeline.informer.util;

import org.apache.log4j.Category;

import java.io.UnsupportedEncodingException;

/**
 * Created by: Serge Lugovoy
 * Date: 08.08.2003
 * Time: 18:13:57
 */
public class GSM7CharSet {

  private static final Category Logger = Category.getInstance(GSM7CharSet.class);

  /**
   * Convert string compatible with ISO8859-1 charset to GSM default alphabet.
   * Each GSM symbol placed in one octet, high bit is set to 0
   *
   * @param msg string to convert
   * @return array of bytes containing text in GSM coding
   * @throws java.io.UnsupportedEncodingException
   *          locale is not supported
   */
  public static byte[] encode(String msg) throws UnsupportedEncodingException {
    byte b[] = msg.getBytes("ISO8859_1");
    byte b7[] = new byte[b.length * 2];
    int k = 0;
    for (int i = 0; i < b.length; i++) {
      switch (b[i]) {
        case '^':
          b7[k++] = 0x1b;
          b7[k++] = 0x14;
          break;
        case '\f':
          b7[k++] = 0x1b;
          b7[k++] = 0x0a;
          break;
        case '|':
          b7[k++] = 0x1b;
          b7[k++] = 0x40;
          break;
        case '{':
          b7[k++] = 0x1b;
          b7[k++] = 0x28;
          break;
        case '}':
          b7[k++] = 0x1b;
          b7[k++] = 0x29;
          break;
        case '[':
          b7[k++] = 0x1b;
          b7[k++] = 0x3c;
          break;
        case ']':
          b7[k++] = 0x1b;
          b7[k++] = 0x3e;
          break;
        case '~':
          b7[k++] = 0x1b;
          b7[k++] = 0x3d;
          break;
        case '\\':
          b7[k++] = 0x1b;
          b7[k++] = 0x2f;
          break;
        default:
          try {
            b7[k++] = _8bit_2_7bit[((int) b[i]) & 0xff];
          } catch (ArrayIndexOutOfBoundsException ex) {
            Logger.warn("GSM7CharSet: index out of bounds: b.len=" + b.length + " i=" + i + " b7.len=" + b7.length + " k=" + k);
          }
      }
    }
    byte out[] = new byte[k];
    System.arraycopy(b7, 0, out, 0, k);
    return out;
  }

  /**
   * Convert byte array that contains GSM default alphabet symbols in each octet
   * to string using ISO8859-1 charset.
   *
   * @param in byte array with GSM symbols
   * @return string containung unicode text
   * @throws java.io.UnsupportedEncodingException
   *          locale is not supported
   */
  public static String decode(byte in[]) throws UnsupportedEncodingException {
    int k = 0;
    byte out[] = new byte[in.length];
    for (int i = 0; i < in.length; ++i) {
      if ((in[i] & 0x7f) == 0x1b) {
        if (++i >= in.length)
          throw new UnsupportedEncodingException("incorrect input buffer, escape symbol is last in buffer");
        switch (in[i] & 0x7f) {
          case 0x0a: // page break
            out[k++] = '\f';
            break;
          case 0x14: //
            out[k++] = '^';
            break;
          case 0x1b: // ??? national
            out[k++] = '?';
            break;
          case 0x40:
            out[k++] = '|';
            break;
          case 0x28: // {
            out[k++] = '{';
            break;
          case 0x29: // }
            out[k++] = '}';
            break;
          case 0x3c: // [
            out[k++] = '[';
            break;
          case 0x3d: // ~
            out[k++] = '~';
            break;
          case 0x3e: // ]
            out[k++] = ']';
            break;
          case 0x2f:
            out[k++] = '\\';
            break;
        }
      } else {
        out[k++] = _7bit_2_8bit[in[i] & 0x7f];
      }
    }
    return new String(out, 0, k, "ISO8859_1");
  }

  private static final byte[] _8bit_2_7bit = {
      0x60, // null
      0x60,
      0x60,
      0x60,
      0x60,
      0x60,
      0x60,
      0x60,
      0x60,
      0x60,
      0xa, // LF
      0x60,
      0x60, // page break
      0xd,
      0x60,
      0x60,
      0x60,
      0x60,
      0x60,
      0x60,
      0x60,
      0x60,
      0x60,
      0x60,
      0x60,
      0x60,
      0x60,
      0x60,
      0x60,
      0x60,
      0x60,
      0x60,
      0x20, // space
      0x21,
      0x22,
      0x23,
      0x2,
      0x25,
      0x26,
      0x27,
      0x28,
      0x29,
      0x2a,
      0x2b,
      0x2c,
      0x2d,
      0x2e,
      0x2f,
      0x30,
      0x31,
      0x32,
      0x33,
      0x34,
      0x35,
      0x36,
      0x37,
      0x38,
      0x39,
      0x3a,
      0x3b,
      0x3c,
      0x3d,
      0x3e,
      0x3f,
      0x0,
      0x41,
      0x42,
      0x43,
      0x44,
      0x45,
      0x46,
      0x47,
      0x48,
      0x49,
      0x4a,
      0x4b,
      0x4c,
      0x4d,
      0x4e,
      0x4f,
      0x50,
      0x51,
      0x52,
      0x53,
      0x54,
      0x55,
      0x56,
      0x57,
      0x58,
      0x59,
      0x5a,
      0x60,
      0x60,
      0x60,
      0x60,
      0x11,
      0x27,
      0x61,
      0x62,
      0x63,
      0x64,
      0x65,
      0x66,
      0x67,
      0x68,
      0x69,
      0x6a,
      0x6b,
      0x6c,
      0x6d,
      0x6e,
      0x6f,
      0x70,
      0x71,
      0x72,
      0x73,
      0x74,
      0x75,
      0x76,
      0x77,
      0x78,
      0x79,
      0x7a,
      0x60,
      0x60,
      0x60,
      0x60,
      0x60,
      0x60,
      0x60,
      0x60,
      0x60,
      0x60,
      0x60,
      0x60,
      0x60,
      0x60,
      0x60,
      0x60,
      0x60,
      0x60,
      0x60,
      0x60,
      0x60,
      0x60,
      0x60,
      0x60,
      0x60,
      0x60,
      0x60,
      0x60,
      0x60,
      0x60,
      0x60,
      0x60,
      0x60,
      0x60,
      0x60,
      0x60,
      0x60,
      0x60,
      0x40,
      0x60,
      0x1,
      0x24,
      0x3,
      0x60,
      0x5f,
      0x60,
      0x60,
      0x60,
      0x60,
      0x60,
      0x60,
      0x60,
      0x60,
      0x60,
      0x60,
      0x60,
      0x60,
      0x60,
      0x60,
      0x60,
      0x60,
      0x60,
      0x60,
      0x60,
      0x60,
      0x60,
      0x60,
      0x60,
      0x60,
      0x60,
      0x60,
      0x60,
      0x60,
      0x5b,
      0xe,
      0x1c,
      0x9,
      0x60,
      0x1f,
      0x60,
      0x60,
      0x60,
      0x60,
      0x60,
      0x60,
      0x60,
      0x5d,
      0x60,
      0x60,
      0x60,
      0x60,
      0x5c,
      0x60,
      0xb,
      0x60,
      0x60,
      0x60,
      0x5e,
      0x60,
      0x60,
      0x1e,
      0x7f,
      0x60,
      0x60,
      0x60,
      0x7b,
      0xf,
      0x1d,
      0x60,
      0x4,
      0x5,
      0x60,
      0x60,
      0x7,
      0x60,
      0x60,
      0x60,
      0x60,
      0x7d,
      0x8,
      0x60,
      0x60,
      0x60,
      0x7c,
      0x60,
      0xc,
      0x6,
      0x60,
      0x60,
      0x7e,
      0x60,
      0x60,
      0x60};

  private static final byte[] _7bit_2_8bit = {
      (byte) 0x40,
      (byte) 0xa3,
      (byte) 0x24,
      (byte) 0xa5,
      (byte) 0xe8,
      (byte) 0xe9,
      (byte) 0xf9,
      (byte) 0xec,
      (byte) 0xf2,
      (byte) 0xc7,
      (byte) 0xa,
      (byte) 0xd8,
      (byte) 0xf8,
      (byte) 0xd,
      (byte) 0xc5,
      (byte) 0xe5,
      (byte) 0x44,
      (byte) 0x5f,
      (byte) 0x46,
      (byte) 0x47,
      (byte) 0x4c,
      (byte) 0x57,
      (byte) 0x50,
      (byte) 0x59,
      (byte) 0x53,
      (byte) 0x51,
      (byte) 0x58,
      (byte) 0x0,
      (byte) 0xc6,
      (byte) 0xe6,
      (byte) 0xdf,
      (byte) 0xc9,
      (byte) 0x20,
      (byte) 0x21,
      (byte) 0x22,
      (byte) 0x23,
      (byte) 0xa4,
      (byte) 0x25,
      (byte) 0x26,
      (byte) 0x27,
      (byte) 0x28,
      (byte) 0x29,
      (byte) 0x2a,
      (byte) 0x2b,
      (byte) 0x2c,
      (byte) 0x2d,
      (byte) 0x2e,
      (byte) 0x2f,
      (byte) 0x30,
      (byte) 0x31,
      (byte) 0x32,
      (byte) 0x33,
      (byte) 0x34,
      (byte) 0x35,
      (byte) 0x36,
      (byte) 0x37,
      (byte) 0x38,
      (byte) 0x39,
      (byte) 0x3a,
      (byte) 0x3b,
      (byte) 0x3c,
      (byte) 0x3d,
      (byte) 0x3e,
      (byte) 0x3f,
      (byte) 0xa1,
      (byte) 0x41,
      (byte) 0x42,
      (byte) 0x43,
      (byte) 0x44,
      (byte) 0x45,
      (byte) 0x46,
      (byte) 0x47,
      (byte) 0x48,
      (byte) 0x49,
      (byte) 0x4a,
      (byte) 0x4b,
      (byte) 0x4c,
      (byte) 0x4d,
      (byte) 0x4e,
      (byte) 0x4f,
      (byte) 0x50,
      (byte) 0x51,
      (byte) 0x52,
      (byte) 0x53,
      (byte) 0x54,
      (byte) 0x55,
      (byte) 0x56,
      (byte) 0x57,
      (byte) 0x58,
      (byte) 0x59,
      (byte) 0x5a,
      (byte) 0xc4,
      (byte) 0xd6,
      (byte) 0xd1,
      (byte) 0xdc,
      (byte) 0xa7,
      (byte) 0xbf,
      (byte) 0x61,
      (byte) 0x62,
      (byte) 0x63,
      (byte) 0x64,
      (byte) 0x65,
      (byte) 0x66,
      (byte) 0x67,
      (byte) 0x68,
      (byte) 0x69,
      (byte) 0x6a,
      (byte) 0x6b,
      (byte) 0x6c,
      (byte) 0x6d,
      (byte) 0x6e,
      (byte) 0x6f,
      (byte) 0x70,
      (byte) 0x71,
      (byte) 0x72,
      (byte) 0x73,
      (byte) 0x74,
      (byte) 0x75,
      (byte) 0x76,
      (byte) 0x77,
      (byte) 0x78,
      (byte) 0x79,
      (byte) 0x7a,
      (byte) 0xe4,
      (byte) 0xf6,
      (byte) 0xf1,
      (byte) 0xfc,
      (byte) 0xe0};

}
