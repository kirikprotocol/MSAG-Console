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

    public ExportSettings(String source, String driver, String user, String password, String tablesPrefix) {
        this.source = source;  this.driver = driver;
        this.user = user; this.password = password;
        this.tablesPrefix = tablesPrefix;
    }

    public String getTablesPrefix() {
        return tablesPrefix;
    }
    public String getSource() {
        return source;
    }
    public String getDriver() {
        return driver;
    }
    public String getUser() {
        return user;
    }
    public String getPassword() {
        return password;
    }
}
