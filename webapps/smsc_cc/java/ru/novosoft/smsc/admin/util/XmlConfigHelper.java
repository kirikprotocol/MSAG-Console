package ru.novosoft.smsc.admin.util;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.filesystem.FileSystem;
import ru.novosoft.smsc.util.config.XmlConfigException;
import ru.novosoft.smsc.util.config.XmlConfig;

import java.io.File;
import java.io.IOException;
import java.io.OutputStream;
import java.text.SimpleDateFormat;
import java.util.Date;

/**
 * @author Artem Snopkov
 */
public class XmlConfigHelper {

  private static final SimpleDateFormat sdf = new SimpleDateFormat("dd.MM.yy.HH.mm.ss");

  public static void saveXmlConfig(XmlConfig config, File toFile, File backupDir, FileSystem fileSystem) throws AdminException {

    File backupFile = new File(backupDir, "configFile.xml." + sdf.format(new Date()));
    fileSystem.copy(toFile, backupFile);

    File tmp = new File(toFile.getAbsolutePath() + ".tmp");
    OutputStream os = null;
    try {
      os = fileSystem.getOutputStream(tmp);
      config.save(os);
    } catch (XmlConfigException e) {
      throw new AdminException(e.getMessage(), e);
    } finally {
      if (os != null) {
        try {
          os.close();
        } catch (IOException e) {
        }
      }
    }

    fileSystem.rename(tmp, toFile);
  }
}
