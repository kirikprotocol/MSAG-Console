/*
 * Created by igork
 * Date: Apr 11, 2002
 * Time: 5:26:52 PM
 */
package ru.novosoft.smsc.util;


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
    String result = "";
    for (int i = 0; i < str.length(); i++) {
      switch (str.charAt(i)) {
        case '<': // &lt;
          result += "&lt;";
          break;
        case '>': //&gt;
          result += "&gt;";
          break;
        case '&': //&amp;
          result += "&amp;";
          break;
        case '"': //&quot;
          result += "&quot;";
          break;
        default:
          result += str.charAt(i);
      }
    }
    return result;
  }
}
