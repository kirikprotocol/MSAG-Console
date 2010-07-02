package ru.novosoft.smsc.admin.filesystem;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.util.Functions;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;

/**
 * author: @author Aleksandr Khalitov
 */
class FileSystemSingleHA extends FileSystem {

  @Override
  public OutputStream getOutputStream(File file) throws AdminException {
    assert file != null : "Some arguments are null";        // todo почему assert? Это же публичный метод. Тет правильнее IllegalArgument кидать или NullPointer
                                                            // todo assert надо кидать в приватных методах.  
    try {
      return new FileOutputStream(file);
    } catch (IOException e) {
      logger.error(e, e);
      throw new AdminException(e.getMessage());
    }
  }

  @Override
  public void rename(File file, File toFile) throws AdminException {
    assert file != null && toFile != null : "Some arguments are null";
    if (!file.renameTo(toFile)) {
      String err = "Can't rename file '" + file.getAbsolutePath() + "' to '" + toFile.getAbsolutePath() + '\'';
      logger.error(err);
      throw new AdminException(err);
    }
  }

  @Override
  public void copy(File file, File toFile) throws AdminException {
    assert file != null && toFile != null : "Some arguments are null";
    try {
      Functions.copyFileTo(file, toFile);
    } catch (IOException e) {
      logger.error(e, e);
      throw new AdminException(e.getMessage());
    }
  }

  @Override
  public void delete(File file) throws AdminException {
    assert file != null : "Some arguments are null";
    if (!file.delete() && file.exists()) {
      String err = "Can't remove file '" + file.getAbsolutePath() + '\'';
      logger.error(err);
      throw new AdminException(err);
    }

  }

  @Override
  public void mkdirs(File file) throws AdminException {
    assert file != null : "Some arguments are null";
    if (!file.mkdirs() && !file.exists()) {
      String err = "Can't create dirs for '" + file.getAbsolutePath() + '\'';
      logger.error(err);
      throw new AdminException(err);
    }
  }

  @Override
  public boolean exists(File file) throws AdminException {
    assert file != null : "Some arguments are null";
    return file.exists();
  }
}
