package ru.novosoft.smsc.admin.resource;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.util.ValidationHelper;

/**
 * @author Artem Snopkov
 */
public class ResourceSettings {

  private static final ValidationHelper vh = new ValidationHelper(ResourceSettings.class);

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

  public void setDecimalDelimiter(String decimalDelimiter) throws AdminException {
    vh.checkNotEmpty("decimalDelimiter", decimalDelimiter);
    this.decimalDelimiter = decimalDelimiter;
  }

  public String getListDelimiter() {
    return listDelimiter;
  }

  public void setListDelimiter(String listDelimiter) throws AdminException {
    vh.checkNotEmpty("listDelimiter", listDelimiter);
    this.listDelimiter = listDelimiter;
  }

  public Resources getResources() {
    return resources;
  }

  public void setResources(Resources resources) {
    this.resources = resources;
  }
}
