package ru.novosoft.smsc.admin.config;

import ru.novosoft.smsc.admin.AdminException;

import java.io.*;

/**
 * @author Artem Snopkov
 */
public class ManagedConfigHelper {

  public static <C> C loadConfig(File configFile, ManagedConfigFile<C> cfg) throws Exception {
    InputStream is = null;
    C c;
    try {
      is = new FileInputStream(configFile);
      c = cfg.load(is);
    } finally {
      if (is != null)
        is.close();
    }
    return c;
  }

  public static <C> void saveConfig(File configFile, ManagedConfigFile<C> cfg, C c) throws Exception {
    InputStream is = null;
    OutputStream os = null;
    File tmpConfigFile = new File(configFile.getAbsolutePath() + ".tmp");
    try {
      is = new FileInputStream(configFile);
      os = new FileOutputStream(tmpConfigFile);

      cfg.save(is, os, c);
    } finally {
      if (is != null)
        try {
          is.close();
        } catch (IOException ignored) {
        }

      if (os != null)
        try {
          os.close();
        } catch (IOException ignored) {
        }
    }

    configFile.delete();
    tmpConfigFile.renameTo(configFile);
  }
}
