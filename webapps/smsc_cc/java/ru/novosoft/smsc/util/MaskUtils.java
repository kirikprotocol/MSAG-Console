package ru.novosoft.smsc.util;

import ru.novosoft.smsc.admin.AdminException;

import java.util.regex.Pattern;

/**
 * Утилиты для работы с масками и адресами
 *
 * @author Aleksandr Khalitov
 */

public class MaskUtils {

  private static final String pattern_header = "^((\\.[0-6]\\.(0|1|3|4|6|8|9|10|14|18)\\.)|(\\+))?";
  private static final Pattern pattern1 = Pattern.compile(pattern_header + "\\d{0,20}\\?{0,20}$");
  private static final Pattern pattern2 = Pattern.compile(pattern_header + "(\\d|\\?){1,20}$");
  private static final String special_pattern_header = "^\\.5\\.[0-6]\\.";
  private static final Pattern special_pattern1 = Pattern.compile(special_pattern_header + "[ _@\\-:\\.\\,0-9A-Za-z]{0,20}\\?{0,11}$");
  private static final Pattern special_pattern2 = Pattern.compile(special_pattern_header + "([ _@\\-:\\.\\,0-9A-Za-z]|\\?){1,11}$");
  private static final Pattern special_pattern3 = Pattern.compile(special_pattern_header + "ussd:([ _@\\-:\\.\\,0-9A-Za-z]|\\?){1,11}$");


  public static MaskInfo parseMask(String mask) throws AdminException {
    if (mask.startsWith(".")) {
      int dp = mask.indexOf('.', 1);
      int dp2 = mask.indexOf('.', dp + 1);
      if (dp < 0 || dp2 < 0)
        throw new AdminException("Mask \"" + mask + "\" is not valid");

      String toneStr = mask.substring(1, dp);
      String npiStr = mask.substring(dp + 1, dp2);
      try {
        return new MaskInfo(Byte.decode(toneStr), Byte.decode(npiStr), mask.substring(dp2 + 1));
      } catch (NumberFormatException e) {
        throw new AdminException("Mask \"" + mask + "\" is not valid, nested: " + e.getMessage());
      }
    } else if (mask.startsWith("+")) {
      return new MaskInfo((byte) 1, (byte) 1, mask.substring(1));
    } else {
      return new MaskInfo((byte) 0, (byte) 1, mask);
    }
  }

  public static String getMask(byte tone, byte npi, String mask) {
    if (tone == 1 && npi == 1)
      return "+" + mask;
    else if (tone == 0 && npi == 1)
      return (mask);
    else
      return getNormalizedMask(tone, npi, mask);
  }

  public static String getNormalizedMask(byte tone, byte npi, String mask) {
    return "." + tone + "." + npi + "." + mask;
  }

  public static boolean isMaskValid(String maskStr) {

    return maskStr != null && maskStr.trim().length() > 0
        && ((pattern1.matcher(maskStr).matches() && pattern2.matcher(maskStr).matches())
        || (special_pattern1.matcher(maskStr).matches() && special_pattern2.matcher(maskStr).matches())
        || special_pattern3.matcher(maskStr).matches());
  }

  public static class MaskInfo {
    private byte tone;
    private byte npi;
    private String mask;

    public MaskInfo(byte tone, byte npi, String mask) {
      this.tone = tone;
      this.npi = npi;
      this.mask = mask;
    }

    public byte getTone() {
      return tone;
    }

    public byte getNpi() {
      return npi;
    }

    public String getMask() {
      return mask;
    }
  }
}
