package ru.novosoft.smsc.infosme.backend.radixtree;

/**
 * Date: August 15, 2007
 * @author artem
 */
public class TemplatesComparator implements CharArraysComparator {
  
  public void compare(char[] chars1, char[] chars2, CharArraysCompareResult result) {
    int i;
    for (i=0; i<chars1.length && i < chars2.length && compareChars(chars1[i], chars2[i]); i++);
    
    if (i > 0) {
      result.commonPrefix = new char[i];
      result.syffix1 = new char[chars1.length - i];
      result.syffix2 = new char[chars2.length - i];
      
      for (int j=0; j < Math.max(chars1.length, chars2.length); j++) {
        if (j < i)
          result.commonPrefix[j] = getMaxChar(chars1[j], chars2[j]);
        else {
          if (j < chars1.length)
            result.syffix1[j-i] = chars1[j];
          if (j < chars2.length)
            result.syffix2[j-i] = chars2[j];
        }
      }
    } else {
      result.commonPrefix = null;
      result.syffix1 = chars1;
      result.syffix2 = chars2;
    }
  }
  
  private static boolean compareChars(char ch1, char ch2) {
    return ch1 == ch2 || ch1 == '?' || ch2 == '?';
  }
  
  private static char getMaxChar(char ch1, char ch2) {
    return (ch1 == '?') ? ch1 : ch2;
  }
  
}
