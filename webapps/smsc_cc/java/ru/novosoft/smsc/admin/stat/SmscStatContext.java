package ru.novosoft.smsc.admin.stat;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.filesystem.FileSystem;

import java.io.File;

/**
 * author: Aleksandr Khalitov
 */
public interface SmscStatContext {

  public File[] getStatDirs() throws AdminException;

  public FileSystem getFileSystem();

}
