/*
 * Created by igork
 * Date: Apr 11, 2002
 * Time: 5:26:52 PM
 */
package ru.novosoft.smsc.util;


public class StringEncoderDecoder
{
  protected static final char ESCAPE_CHAR = '_';
  public static String encode(String str)
  {
    String result = "";
    for (int i = 0; i < str.length(); i++)
    {
      char c = str.charAt(i);
      if (Character.isLetterOrDigit(c))
      {
        result += c;
      }
      else
      {
        String cs = Integer.toHexString(Character.getNumericValue(c));
        result += ESCAPE_CHAR + (cs.length() == 1 ? "0" + cs : cs);
      }
    }
    return result;
  }

  public static String decode(String str)
  {
    String result = "";
    int pos = 0;
    int curpos = str.indexOf(ESCAPE_CHAR);
    while (curpos > 0) {
      result += str.substring(pos, curpos);
      pos = curpos + 3;
      result += (char)Integer.parseInt(str.substring(curpos +1, curpos +3), 16);
      curpos = str.indexOf(ESCAPE_CHAR, pos);
    }
    result += str.substring(pos);
    return result;
  }
}
