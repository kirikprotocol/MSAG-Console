package mobi.eyeline.util.jsf.components.sample.controllers;

import javax.faces.context.FacesContext;
import java.io.*;

/**
 * author: Aleksandr Khalitov
 */
public class FileDownloadingController {

  private String text;

  public String getText() {
    return text;
  }

  public void setText(String text) {
    this.text = text;
  }

  public void download(FacesContext context, OutputStream os) {
    PrintWriter writer = null;
    try{
      writer = new PrintWriter(new BufferedWriter(new OutputStreamWriter(os, "windows-1251")));
      if(text != null && text.length()>0) {
        writer.println(text);
      }else {
        writer.println("empty");
      }
    } catch (UnsupportedEncodingException e) {
      e.printStackTrace();
    } finally {
      if(writer != null) {
        writer.close();
      }
    }
  }

}
