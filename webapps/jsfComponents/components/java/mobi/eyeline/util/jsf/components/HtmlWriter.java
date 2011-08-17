package mobi.eyeline.util.jsf.components;

import javax.faces.context.ResponseWriter;
import java.io.IOException;

/**
 * User: artem
 * Date: 26.07.11
 */
public class HtmlWriter {

  private ResponseWriter w;

  public HtmlWriter(ResponseWriter w) {
    this.w = w;
  }

  public HtmlWriter a(String s) throws IOException {
    w.append(s);
    return this;
  }

  public HtmlWriter a(char s) throws IOException {
    w.append(s);
    return this;
  }

}
