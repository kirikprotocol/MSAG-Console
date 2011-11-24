package ru.novosoft.smsc.admin.mcisme;

import ru.novosoft.smsc.admin.config.ManagedConfigFile;
import ru.novosoft.smsc.util.config.XmlConfig;

import java.io.InputStream;
import java.io.OutputStream;

/**
 * author: Aleksandr Khalitov
 */
public class MCISmeConfig implements ManagedConfigFile<MCISmeSettings> {
  public void save(InputStream oldFile, OutputStream newFile, MCISmeSettings conf) throws Exception {



    //todo
  }

  public MCISmeSettings load(InputStream is) throws Exception {
    XmlConfig c = new XmlConfig();
    c.load(is);

    MCISmeSettings s = new MCISmeSettings();




    return s;  //todo
  }
}
