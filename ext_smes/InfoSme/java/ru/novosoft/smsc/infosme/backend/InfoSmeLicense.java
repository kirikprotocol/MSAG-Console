package ru.novosoft.smsc.infosme.backend;

import org.apache.log4j.Category;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.Properties;

/**
 * author: alkhal
 * @noinspection EmptyCatchBlock
 */
public class InfoSmeLicense {

  private static final Category category = Category.getInstance(InfoSmeLicense.class);

  private static InfoSmeLicense instance;

  private final File file;

  private final Properties properties = new Properties();

  private synchronized static void init(File file) throws IOException {
    if(instance == null) {
      instance = new InfoSmeLicense(file);
    }
  }

  InfoSmeLicense(File f) throws IOException{
    if(f == null || !f.exists()) {
      throw new IllegalArgumentException("License is not found: "+(f == null ? null : f.getAbsolutePath()));
    }
    this.file = f;
    InputStream is = null;
    try{
      is = new FileInputStream(f);
      properties.load(is);
    } finally {
      if(is != null) {
        try{
          is.close();
        }catch (IOException e){}
      }
    }
  }

  public String getProperty(String name) {
    return properties.getProperty(name);
  }

  public boolean contains(String name) {
    return properties.containsKey(name);
  }  


}
