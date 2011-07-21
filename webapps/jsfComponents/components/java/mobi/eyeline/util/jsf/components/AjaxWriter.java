package mobi.eyeline.util.jsf.components;

import javax.faces.component.UIComponent;
import javax.faces.context.ResponseWriter;
import java.io.IOException;
import java.io.Writer;

/**
 * User: artem
 * Date: 21.07.11
 */
public class AjaxWriter extends ResponseWriter {

  private boolean skipContent = true;

  private ResponseWriter w;

  public AjaxWriter(ResponseWriter w) {
    this.w = w;
  }

  public boolean isSkipContent() {
    return skipContent;
  }

  public void setSkipContent(boolean skipContent) {
    this.skipContent = skipContent;
  }

  @Override
  public String getContentType() {
    return w.getContentType();
  }

  @Override
  public String getCharacterEncoding() {
    return w.getCharacterEncoding();
  }

  @Override
  public void flush() throws IOException {
    w.flush();
  }

  @Override
  public void startDocument() throws IOException {
    if(skipContent) {
      return;
    }
    w.startDocument();
  }

  @Override
  public void endDocument() throws IOException {
    if(skipContent) {
      return;
    }
    w.endDocument();
  }

  @Override
  public void startElement(String s, UIComponent uiComponent) throws IOException {
    if(skipContent) {
      return;
    }
    w.startElement(s, uiComponent);
  }

  @Override
  public void endElement(String s) throws IOException {
    if(skipContent) {
      return;
    }
    w.endElement(s);
  }

  @Override
  public void writeAttribute(String s, Object o, String s1) throws IOException {
    if(skipContent) {
      return;
    }
    w.writeAttribute(s, o, s1);
  }

  @Override
  public void writeURIAttribute(String s, Object o, String s1) throws IOException {
    if(skipContent) {
      return;
    }
    w.writeURIAttribute(s, o, s1);
  }

  @Override
  public void writeComment(Object o) throws IOException {
    if(skipContent) {
      return;
    }
    w.writeComment(o);
  }

  @Override
  public void writeText(Object o, String s) throws IOException {
    if(skipContent) {
      return;
    }
    w.writeText(o, s);
  }

  @Override
  public void writeText(char[] chars, int i, int i1) throws IOException {
    if(skipContent) {
      return;
    }
    w.writeText(chars, i, i1);
  }

  @Override
  public void writeText(Object object, UIComponent component, String string) throws IOException {
    if(skipContent) {
      return;
    }
    w.writeText(object, component, string);
  }

  @Override
  public void write(int c) throws IOException {
    if(skipContent) {
      return;
    }
    w.write(c);
  }

  @Override
  public void write(char[] cbuf) throws IOException {
    if(skipContent) {
      return;
    }
    w.write(cbuf);
  }

  @Override
  public void write(char[] cbuf, int off, int len) throws IOException {
    if(skipContent) {
      return;
    }
    w.write(cbuf, off, len);
  }

  @Override
  public void write(String str) throws IOException {
    if(skipContent) {
      return;
    }
    w.write(str);
  }

  @Override
  public void write(String str, int off, int len) throws IOException {
    if(skipContent) {
      return;
    }
    w.write(str, off, len);
  }

  @Override
  public Writer append(CharSequence csq) throws IOException {
    if(skipContent) {
      return this;
    }
    return w.append(csq);
  }

  @Override
  public Writer append(CharSequence csq, int start, int end) throws IOException {
    if(skipContent) {
      return this;
    }
    return w.append(csq, start, end);
  }

  @Override
  public Writer append(char c) throws IOException {
    if(skipContent) {
      return this;
    }
    return w.append(c);
  }

  @Override
  public void close() throws IOException {
    w.close();
  }

  @Override
  public ResponseWriter cloneWithWriter(Writer writer) {
    return this;
  }
}
