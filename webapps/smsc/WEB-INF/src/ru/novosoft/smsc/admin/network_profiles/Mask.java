/*
 * Author: igork
 * Date: 14.05.2002
 * Time: 13:37:28
 */
package ru.novosoft.smsc.admin.network_profiles;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.util.StringEncoderDecoder;

import java.io.PrintWriter;


public class Mask implements Comparable
{
  private static final String pattern_header = "^((\\.[0-6]\\.(0|1|3|4|6|8|9|10|14|18)\\.)|(\\+))?";
  private static final String pattern1 = pattern_header + "\\d{0,20}\\?{0,20}$";
  private static final String pattern2 = pattern_header + "(\\d|\\?){1,20}$";
  private static final String special_pattern_header = "^\\.5\\.[0-6]\\.";
  private static final String special_pattern1 = special_pattern_header + "[ _@\\-:\\.\\,0-9A-Za-z]{0,20}\\?{0,11}$";
  private static final String special_pattern2 = special_pattern_header + "([ _@\\-:\\.\\,0-9A-Za-z]|\\?){1,11}$";
  private static final String special_pattern3 = special_pattern_header + "ussd:([ _@\\-:\\.\\,0-9A-Za-z]|\\?){1,11}$";

  private byte tone = 0;
  private byte npi = 0;
  private String mask = null;

  public Mask(byte tone, byte npi, String mask) {
    this.tone = tone;
    this.npi = npi;
    this.mask = mask;
  }

  public Mask(String mask) throws AdminException
  {
    String tmpMask = mask.trim();
    if (tmpMask == null)
      throw new NullPointerException("Mask string is null");
    if (!isMaskValid(tmpMask))
      throw new AdminException("Mask \"" + tmpMask + "\" is not valid");
    parseMask(tmpMask);
  }

  private void parseMask(String mask) throws AdminException
  {
    if (mask.startsWith(".")) {
      int dp = mask.indexOf('.', 1);
      int dp2 = mask.indexOf('.', dp + 1);
      if (dp < 0 || dp2 < 0)
        throw new AdminException("Mask \"" + mask + "\" is not valid");

      String toneStr = mask.substring(1, dp);
      String npiStr = mask.substring(dp + 1, dp2);
      try {
        this.tone = Byte.decode(toneStr).byteValue();
        this.npi = Byte.decode(npiStr).byteValue();
        this.mask = mask.substring(dp2 + 1);
      } catch (NumberFormatException e) {
        throw new AdminException("Mask \"" + mask + "\" is not valid, nested: " + e.getMessage());
      }
    } else if (mask.startsWith("+")) {
      this.tone = 1;
      this.npi = 1;
      this.mask = mask.substring(1);
    } else {
      this.tone = 0;
      this.npi = 1;
      this.mask = mask;
    }
  }

  public String getMaskSimple() {
    return mask;
  }

  public String getMask()
  {
    if (tone == 1 && npi == 1)
      return "+" + mask;
    else if (tone == 0 && npi == 1)
      return (mask);
    else
      return getNormalizedMask();
  }

  public String getNormalizedMask()
  {
    return "." + tone + "." + npi + "." + mask;
  }

  public boolean equals(Object obj)
  {
    if (obj instanceof Mask) {
      Mask m = (Mask) obj;
      return m.tone == this.tone && m.npi == this.npi && m.mask.equals(this.mask);
    } else
      return false;
  }

  public PrintWriter store(PrintWriter out)
  {
    out.println("       <mask value=\"" + StringEncoderDecoder.encode(getNormalizedMask()) + "\"/>");
    return out;
  }

  public static boolean isMaskValid(String maskStr)
  {
    // valid masks:
    // 	.[0..6].[0|1|3|4|6|8|9|10|14|18].[x]
    // 	+[x]	==> .1.1.[x]
    // 	[x]	==> .0.1.[x]
    // ��� x - numeric string ������ �� 1 �� 21

    return maskStr != null && maskStr.trim().length() > 0
            && ((maskStr.matches(pattern1) && maskStr.matches(pattern2))
            || (maskStr.matches(special_pattern1) && maskStr.matches(special_pattern2))
	    || maskStr.matches(special_pattern3));
  }

  public int getQuestionsCount()
  {
    int pos = mask.indexOf('?');
    if (pos >= 0)
      return mask.length() - pos;
    else
      return 0;
  }

  public byte getTone()
  {
    return tone;
  }

  public byte getNpi()
  {
    return npi;
  }

  public boolean isMask()
  {
    return mask != null && mask.endsWith("?");
  }

  public boolean addressConfirm(String address) throws AdminException
  {
    return addressConfirm(new Mask(address));
  }


  public boolean addressConfirm(Mask address)
  {
    return address.tone == tone
            && address.npi == npi
            && mask.length() == address.mask.length()
            && address.mask.startsWith(mask.substring(0, mask.length() - getQuestionsCount()));
  }

  public int compareTo(Object o) {
    if(o == null || !(o instanceof Mask)) {
      return 1;
    }
    return getMaskSimple().compareTo(((Mask)o).getMaskSimple());
  }
}
