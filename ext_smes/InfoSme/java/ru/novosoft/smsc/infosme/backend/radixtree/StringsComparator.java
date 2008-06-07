package ru.novosoft.smsc.infosme.backend.radixtree;

/**
 * Date: August 15, 2007
 * @author artem
 */
public class StringsComparator implements CharArraysComparator {

  public void compare(char[] bytes1, char[] bytes2, CharArraysCompareResult result) {
    int i;
    for (i = 0; i < bytes1.length && i < bytes2.length && bytes1[i] == bytes2[i]; i++);

    if (i > 0) {
      result.commonPrefix = new char[i];
      result.syffix1 = new char[bytes1.length - i];
      result.syffix2 = new char[bytes2.length - i];

      for (int j = 0; j < Math.max(bytes1.length, bytes2.length); j++) {
        if (j < i) {
          result.commonPrefix[j] = bytes1[j];
        } else {
          if (j < bytes1.length) {
            result.syffix1[j - i] = bytes1[j];
          }
          if (j < bytes2.length) {
            result.syffix2[j - i] = bytes2[j];
          }
        }
      }
    } else {
      result.commonPrefix = null;
      result.syffix1 = bytes1;
      result.syffix2 = bytes2;
    }
  }
}
