/*
 * Author: igork
 * Date: 14.05.2002
 * Time: 13:37:28
 */
package ru.sibinco.lib.backend.route;

import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.backend.util.StringEncoderDecoder;

import java.io.PrintWriter;


public class Mask
{
  private static final String pattern_header = "^((\\.[0-6]\\.(0|1|3|4|6|8|9|10|14|18)\\.)|(\\+))?";
  private static final String pattern1 = pattern_header + "\\d{0,20}\\?{0,20}$";
  private static final String pattern2 = pattern_header + "(\\d|\\?){1,20}$";
  private static final String special_pattern_header = "^\\.5\\.0\\.";
  private static final String special_pattern1 = special_pattern_header + "[\\p{Graph}&&[^\\*\\s\\?\\\\'\"]]{0,20}\\?{0,20}$";
  private static final String special_pattern2 = special_pattern_header + "[\\p{Graph}&&[^\\*\\s\\\\'\"]]{1,20}$";

  private byte tone = 0;
  private byte npi = 0;
  private String mask = null;

  public Mask(String mask) throws SibincoException
  {
    String tmpMask = mask.trim();
    if (tmpMask == null)
      throw new NullPointerException("Mask string is null");
    if (!isMaskValid(tmpMask))
      throw new SibincoException("Mask \"" + tmpMask + "\" is not valid");
    parseMask(tmpMask);
  }

  private void parseMask(String mask) throws SibincoException
  {
    if (mask.startsWith(".")) {
      int dp = mask.indexOf('.', 1);
      int dp2 = mask.indexOf('.', dp + 1);
      if (dp < 0 || dp2 < 0)
        throw new SibincoException("Mask \"" + mask + "\" is not valid");

      String toneStr = mask.substring(1, dp);
      String npiStr = mask.substring(dp + 1, dp2);
      try {
        this.tone = Byte.decode(toneStr).byteValue();
        this.npi = Byte.decode(npiStr).byteValue();
        this.mask = mask.substring(dp2 + 1);
      } catch (NumberFormatException e) {
        throw new SibincoException("Mask \"" + mask + "\" is not valid, nested: " + e.getMessage());
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
    out.println("    <mask value=\"" + StringEncoderDecoder.encode(getNormalizedMask()) + "\"/>");
    return out;
  }

    public PrintWriter store(PrintWriter out, String param) {
        out.println("    <" + param + "  value=\"" + StringEncoderDecoder.encode(getNormalizedMask()) + "\"/>");
        return out;
    }

    public PrintWriter storeNotNormalizedMask(PrintWriter out, String param) {
        out.println("    <" + param + "  value=\"" + StringEncoderDecoder.encode(getMask()) + "\"/>");
        return out;
    }

  public static boolean isMaskValid(String maskStr)
  {
    // valid masks:
    // 	.[0..6].[0|1|3|4|6|8|9|10|14|18].[x]
    // 	+[x]	==> .1.1.[x]
    // 	[x]	==> .0.1.[x]
    // где x - numeric string длиной от 1 до 21

    return maskStr != null && maskStr.trim().length() > 0
           && ((maskStr.matches(pattern1) && maskStr.matches(pattern2))
               || (maskStr.matches(special_pattern1) && maskStr.matches(special_pattern2)));
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

  public boolean addressConfirm(String address) throws SibincoException
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
}
