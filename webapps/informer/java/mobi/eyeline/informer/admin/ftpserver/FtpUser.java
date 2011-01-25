package mobi.eyeline.informer.admin.ftpserver;

/**
 * User: artem
 * Date: 25.01.11
 */
public class FtpUser {
  private final String login;
  private final String password;
  private final int maxDirSize;

  public FtpUser(String login, String password, int maxDirSize) {
    this.login = login;
    this.password = password;
    this.maxDirSize = maxDirSize;
  }

  public String getLogin() {
    return login;
  }

  public String getPassword() {
    return password;
  }

  public int getMaxDirSize() {
    return maxDirSize;
  }
}
