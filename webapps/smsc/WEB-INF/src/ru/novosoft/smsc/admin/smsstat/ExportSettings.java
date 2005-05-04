package ru.novosoft.smsc.admin.smsstat;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 03.05.2005
 * Time: 14:39:03
 * To change this template use File | Settings | File Templates.
 */
public class ExportSettings
{
    private String source = null;
    private String driver = null;
    private String user = null;
    private String password = null;
    private String tablesPrefix = null;

    public ExportSettings() {}
    public ExportSettings(String source, String driver, String user, String password, String tablesPrefix) {
        this.source = source;  this.driver = driver;
        this.user = user; this.password = password;
        this.tablesPrefix = tablesPrefix;
    }

    public boolean isEmpty() {
        return (source == null || source.length() <= 0 || driver == null || driver.length() <= 0 ||
                user == null || user.length() <= 0 || password == null ||
                tablesPrefix == null || tablesPrefix.length() <= 0);
    }
    public String getSource() {
        return source;
    }
    public void setSource(String source) {
        this.source = source;
    }

    public String getDriver() {
        return driver;
    }
    public void setDriver(String driver) {
        this.driver = driver;
    }

    public String getUser() {
        return user;
    }
    public void setUser(String user) {
        this.user = user;
    }

    public String getPassword() {
        return password;
    }
    public void setPassword(String password) {
        this.password = password;
    }

    public String getTablesPrefix() {
        return tablesPrefix;
    }
    public void setTablesPrefix(String tablesPrefix) {
        this.tablesPrefix = tablesPrefix;
    }
}
