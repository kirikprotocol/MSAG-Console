package mobi.eyeline.informer.util;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 20.10.2010
 * Time: 15:20:14
 */
public class CSVTokenizer {
  private final String line;
  private int start = 0;


  public CSVTokenizer(String line) {
    this.line = line;
  }

  public String nextToken() {
    boolean inQuot = false;
    for (int i = start; i < line.length(); i++) {
      char c = line.charAt(i);
      if (c == '\"') {
        inQuot = !inQuot;
      } else {
        if (!inQuot && c == ',') {
          String ret = line.substring(start, i);
          start = i + 1;
          return decodeToken(ret);
        }
      }
    }
    String ret = line.substring(start, line.length());
    start = line.length();
    return decodeToken(ret);
  }

  public boolean hasMoreTokens() {
    return start < line.length();
  }

  private String decodeToken(String s) {
    if (s.startsWith("\"") && s.endsWith("\"")) {
      s = s.substring(1, s.length() - 1);
    }
    return s.replace("\"\"", "\"");
  }

}
