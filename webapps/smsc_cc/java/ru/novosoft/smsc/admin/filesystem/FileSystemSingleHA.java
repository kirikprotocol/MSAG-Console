package ru.novosoft.smsc.admin.filesystem;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.util.FileUtils;

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
    if (file == null)
      throw new NullPointerException("File arg is null");
    try {
      return new FileOutputStream(file);
    } catch (IOException e) {
      throw new FileSystemException("io_error",e);
    }
  }

  @Override
  public void rename(File file, File toFile) throws AdminException {
    if (file == null || toFile == null)
      throw new NullPointerException("File arg is null");

    if (!file.renameTo(toFile)) {
      String err = "Can't rename file '" + file.getAbsolutePath() + "' to '" + toFile.getAbsolutePath() + '\'';
      throw new FileSystemException("io_error",err);
    }
  }

  @Override
  public void copy(File file, File toFile) throws AdminException {
    if (file == null || toFile == null)
      throw new NullPointerException("File arg is null");
    try {
      FileUtils.copyFileTo(file, toFile);
    } catch (IOException e) {
      throw new FileSystemException("io_error",e);
    }
  }

  @Override
  public void delete(File file) throws AdminException {
    if (file == null)
      throw new NullPointerException("File arg is null");
    if (!file.delete() && file.exists()) {
      String err = "Can't remove file '" + file.getAbsolutePath() + '\'';
      throw new FileSystemException("io_error",err);
    }

  }

  @Override
  public void mkdirs(File file) throws AdminException {
    if (file == null)
      throw new NullPointerException("File arg is null");
    if (!file.mkdirs() && !file.exists()) {
      String err = "Can't create dirs for '" + file.getAbsolutePath() + '\'';
      throw new FileSystemException("io_error",err);
    }
  }

  @Override
  public boolean exists(File file) throws AdminException {
    if (file == null)
      throw new NullPointerException("File arg is null");
    return file.exists();
  }
}
