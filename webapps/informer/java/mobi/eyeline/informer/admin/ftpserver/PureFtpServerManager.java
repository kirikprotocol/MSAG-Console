package mobi.eyeline.informer.admin.ftpserver;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.admin.service.ServiceManager;
import org.apache.log4j.Logger;

import java.io.*;
import java.util.Collection;

/**
 * User: artem
 * Date: 25.01.11
 */
public class PureFtpServerManager extends AbstractFtpServerManager {

  private static final Logger log = Logger.getLogger("FTPSERVER");

  private final FileSystem fs;

  public PureFtpServerManager(ServiceManager serviceManager, String serviceId, FileSystem fs) {
    super(serviceManager, serviceId);
    this.fs = fs;
  }

  @Override
  public File getUserHomeDir(String login) throws AdminException {
    return new File(getServerBaseDir(), "users" + File.separator + login);
  }

  @Override
  public void updateUsers(Collection<FtpUser> users) throws AdminException {
    File tmpFile = new File(getServerBaseDir(), "conf" + File.separator + "users.csv");
    try {
      saveUsersToFile(users, tmpFile);
    } catch (IOException e) {
      throw new FtpServerManagerException("pure.ftp.tmp.file.save.err", e.getMessage());
    }

    String command = new File(getServerBaseDir(), "bin" + File.separator + "updateUsers.sh").getAbsolutePath();
    try {
      String result = runCommand(command);
      if (!result.isEmpty())
        throw new FtpServerManagerException("pure.ftp.user.update.failed", result);
    } catch (Exception e) {
      throw new FtpServerManagerException("pure.ftp.user.update.failed", e.getMessage());
    }
  }

  private void saveUsersToFile(Collection<FtpUser> users, File file) throws IOException, AdminException {
    File tmpFile = new File(file.getAbsolutePath()+".tmp");
    BufferedWriter w = null;
    try {
      w = new BufferedWriter(new OutputStreamWriter(fs.getOutputStream(tmpFile, false)));

      for (FtpUser u : users) {
        w.append(u.getLogin()).append(':').append(u.getPassword()).append(':').append(String.valueOf(u.getMaxDirSize()));
        w.newLine();
      }

    } finally {
      if (w != null)
        try {
          w.close();
        } catch (IOException ignored) {
        }
    }

    if (fs.exists(file))
      fs.delete(file);
    fs.rename(tmpFile, file);
  }

  /**
   * Исполняет команду и возвращщает err.
   * @param command команда для исполнения
   * @return Вывод System.err команды
   * @throws InterruptedException если исполнение команды было прервано
   * @throws IOException если не удалось выполнить команду
   */
  private static String runCommand(String command) throws InterruptedException, IOException {
    StringBuilder result = new StringBuilder();
    Process p = Runtime.getRuntime().exec(command);
    BufferedReader in = null;
    BufferedReader err = null;
    try {
      err = new BufferedReader(new InputStreamReader(p.getErrorStream()));
      in = new BufferedReader(new InputStreamReader(p.getInputStream()));
      String linein;
      String lineerr = null;
      while (((linein = in.readLine()) != null) || ((lineerr = err.readLine()) != null)) {
        if (linein != null)
          log.info(linein);

        if (lineerr != null)
          result.append(lineerr).append("\n");
      }
    } finally {
      if (err != null) {
        try {
          err.close();
        } catch (IOException e) {
        }
      }
      if (in != null) {
        try {
          in.close();
        } catch (IOException e) {
        }
      }
    }

    p.waitFor();
    return result.toString();
  }
}
