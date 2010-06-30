package ru.novosoft.smsc.admin.filesystem;

import org.apache.log4j.Category;
import ru.novosoft.smsc.admin.AdminException;

import java.io.*;

/**
 * author: alkhal
 */
public abstract class FileSystem {       //todo java doc

  protected static final Category logger = Category.getInstance(FileSystem.class);

  public static FileSystem getInstance() {
    return null;
  }

  public InputStream getInputStream(File file) throws AdminException{
    try{
      return new FileInputStream(file);
    }catch (IOException e){
      logger.error(e,e);
      throw new AdminException(e.getMessage());
    }
  }

  public abstract OutputStream getOutputStream(File file) throws AdminException;

  public abstract void rename(File file, File toFile) throws AdminException;

  public abstract void copy(File file, File toFile) throws AdminException;

  public abstract void delete(File file) throws AdminException;

  public abstract void mkdirs(File file) throws AdminException;

}
