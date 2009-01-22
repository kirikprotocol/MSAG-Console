package mobi.eyeline.smsquiz.beans.util;

/**
 * author: alkhal
 * Date: 21.01.2009
 */
public class Tokenizer {
  private String str;
  private String delim;
  private String result;

  public Tokenizer(String str, String delim) {
    this.str = str.substring(0);
    this.delim = delim;
    prepare();
  }

  public String next() {
    String toReturn = result.substring(0);
    str = str.substring(str.indexOf(delim)+1);
    prepare();
    return toReturn;
  }

  private void prepare() {
    if(str.indexOf(delim)>-1) {
      result = str.substring(0, str.indexOf(delim));
    } else {
      result = str;
    }
  }
}
