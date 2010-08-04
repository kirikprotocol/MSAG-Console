package ru.novosoft.smsc.admin.resource;

/**
 * @author Artem Snopkov
 */
public class ResourceSettings {

  private String decimalDelimiter;
  private String listDelimiter;
  private Resources resources;

  public ResourceSettings(String decimalDelimiter, String listDelimiter, Resources resources) {
    this.decimalDelimiter = decimalDelimiter;
    this.listDelimiter = listDelimiter;
    this.resources = resources;
  }

  public String getDecimalDelimiter() {
    return decimalDelimiter;
  }

  public String getListDelimiter() {
    return listDelimiter;
  }

  public Resources getResources() {
    return resources;
  }
}
