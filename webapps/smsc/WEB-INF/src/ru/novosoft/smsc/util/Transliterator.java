package ru.novosoft.smsc.util;

/**
 * Created by: Serge Lugovoy
 * Date: 10.06.2004
 * Time: 13:46:01
 */
public class Transliterator
{
  private final static String[] translit = {
    "A", "B", "V", "G", "D", "E", "Zh", "Z", "I", "Y", "K", "L", "M", "N", "O",
    "P", "R", "S", "T", "U", "F", "H", "Ts", "Ch", "Sh", "Sch", "'", "I", "'",
    "E", "Yu", "Ya", "a", "b", "v", "g", "d", "e", "zh", "z", "i", "y", "k",
    "l", "m", "n", "o", "p", "r", "s", "t", "u", "f", "h", "ts", "ch", "sh",
    "sch", "'", "i", "'", "e", "yu", "ya"
  };

  public static String translit(String s)
  {
    if (s == null) return null;

    StringBuffer sb = new StringBuffer();
    for (int i = 0; i < s.length(); i++) {
      char c = s.charAt(i);
      if (1040 <= c && c <= 1103) sb.append(translit[c-1040]);
      else if (c == 1025)         sb.append('E');
      else if (c == 1105)         sb.append('e');
      else                        sb.append(c);
    }
    return sb.toString();
  }

}
