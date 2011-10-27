package ru.novosoft.smsc.admin.stat;

/**
 * author: Aleksandr Khalitov
 */
public class ExportSettings {

  private String source;
  private DbType dbType;
  private String user;
  private String pass;
  private String prefix;

  public ExportSettings(ExportSettings o) {
    source = o.source;
    dbType = o.dbType;
    user = o.user;
    pass = o.pass;
    prefix = o.prefix;
  }

  public ExportSettings() {
  }

  public String getSource() {
    return source;
  }

  public void setSource(String source) {
    this.source = source;
  }

  public DbType getDbType() {
    return dbType;
  }

  public void setDbType(DbType dbType) {
    this.dbType = dbType;
  }

  public String getUser() {
    return user;
  }

  public void setUser(String user) {
    this.user = user;
  }

  public String getPass() {
    return pass;
  }

  public void setPass(String pass) {
    this.pass = pass;
  }

  public String getPrefix() {
    return prefix;
  }

  public void setPrefix(String prefix) {
    this.prefix = prefix;
  }

  public static enum DbType {
    MYSQL, ORACLE
  }
}
