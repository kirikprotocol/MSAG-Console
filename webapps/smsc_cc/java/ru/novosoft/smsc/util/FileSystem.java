package ru.novosoft.smsc.util;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;

/**
 * Прокси для работы с файловой системой
 * @author Artem Snopkov
 */
public abstract class FileSystem {

  public static FileSystem getInstance() {
    return null;
  }

  public abstract FileInputStream getInputStream(String file);
  public abstract FileInputStream getInputStream(File file);

  public abstract FileOutputStream getOutputStream(String file);
  public abstract FileOutputStream getOutputStream(File file);

  public abstract boolean deleteFile(File file);
  public abstract boolean copyFile(File source, File destination);
  public abstract boolean renameFile(File from, File to);
}
