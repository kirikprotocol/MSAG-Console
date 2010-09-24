package mobi.eyeline.informer.admin.config;

/**
 * Структура, содержащая данные о последней ревизии конфига
 * @author Artem Snopkov
 */
public class Revision {
  private String user;
  private int number;

  public Revision(String user, int number) {
    this.user = user;
    this.number = number;
  }

  public String getUser() {
    return user;
  }

  public void setUser(String user) {
    this.user = user;
  }

  public int getNumber() {
    return number;
  }

  public void setNumber(int number) {
    this.number = number;
  }
}
