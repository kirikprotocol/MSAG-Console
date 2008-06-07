package ru.novosoft.smsc.infosme.backend;

import ru.novosoft.smsc.admin.AdminException;

import java.io.RandomAccessFile;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;

/**
 * User: artem
 * Date: 19.05.2008
 */

public class TaskManager {

  private int id;
  private final Object idlock = new Object();
  private RandomAccessFile idFile;

  public TaskManager(String configDir) throws AdminException {
    try {
      idFile = new RandomAccessFile(new File(configDir, "taskid.bin"), "rw");
      long len = idFile.length();
      if (len != 0 && len != 4)
        throw new AdminException("Broken taskid.bin file");
      // Load id
      id = (len == 0) ? 0 : idFile.readInt();
    } catch (IOException e) {
      e.printStackTrace();
      throw new AdminException("Can't init task manager");
    }
  }

  private int getId() throws AdminException {
    synchronized(idlock) {
      try {
        id++;
        idFile.seek(0);
        idFile.writeInt(id);
        return id;
      } catch (IOException e) {
        throw new AdminException(e.getMessage());
      }
    }
  }


  public Task createTask() throws AdminException {
    return new Task(String.valueOf(getId()));
  }
}
