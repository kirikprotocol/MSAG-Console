package mobi.eyeline.util.jsf.components.sample.controllers;

import mobi.eyeline.util.jsf.components.input_file.UploadedFile;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;

/**
 * author: Aleksandr Khalitov
 */
public class InputFileController {

  private UploadedFile value;

  private String descr;

  public String getDescr() {
    return descr;
  }

  public void setDescr(String descr) {
    this.descr = descr;
  }

  public UploadedFile getValue() {
    return value;
  }

  public void setValue(UploadedFile value) {
    this.value = value;
  }

  public String action() {
    if(value != null) {
      descr = "Content-type: "+value.getContentType()+"\n";
      descr += "Lenght: "+value.getLength()+"\n";
      BufferedReader reader = null;
      try{
        reader = new BufferedReader(new InputStreamReader(value.getInputStream()));
        String line = reader.readLine();
        if(line != null) {
          System.out.println(line);
          if(line.length()>100) {
            line = line.substring(0, 100);
          }
          descr+="Start with: "+line+"...";
          while((line = reader.readLine())!= null) {
            System.out.println(line);
          }
        }
      }catch (Exception e) {
        e.printStackTrace();
      }finally {
        if(reader != null) {
          try{
            reader.close();
          }catch (IOException ignored) {}
        }
      }
    }
    return null;
  }
}
