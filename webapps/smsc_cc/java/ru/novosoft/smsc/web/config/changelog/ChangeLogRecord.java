package ru.novosoft.smsc.web.config.changelog;

import javax.faces.context.FacesContext;
import java.util.Locale;
import java.util.ResourceBundle;

/**
 * @author Artem Snopkov
 */
public class ChangeLogRecord {

  private long time;
  private String subjectKey;
  private String user;
  private final Type type;
  private String descriptionKey;
  private String[] descriptionArgs;

  public ChangeLogRecord(Type type) {
    this.type = type;
  }

  public long getTime() {
    return time;
  }

  void setTime(long time) {
    this.time = time;
  }

  public String getSubject() {
    return ResourceBundle.getBundle(ChangeLogRecord.class.getCanonicalName(), getLocale()).getString(subjectKey);
  }

  String getSubjectKey() {
    return subjectKey;
  }

  void setSubjectKey(String subject) {
    this.subjectKey = subject;
  }

  public String getUser() {
    return user;
  }

  void setUser(String user) {
    this.user = user;
  }

  void setDescription(String key, String... args) {
    this.descriptionKey = key;
    this.descriptionArgs = args;
  }

  String getDescriptionKey() {
    return descriptionKey;
  }

  String[] getDescriptionArgs() {
    return descriptionArgs;
  }

  public String getDescription() {
    String str = ResourceBundle.getBundle(ChangeLogRecord.class.getCanonicalName(), getLocale()).getString(descriptionKey);
    if (descriptionArgs != null) {
      for (int i=0; i<descriptionArgs.length; i++)
        str = str.replaceAll("\\{" + i + "\\}", descriptionArgs[i]);
    }
    return str;
  }

  private Locale getLocale() {
    Locale l = Locale.ENGLISH;
    FacesContext fc = FacesContext.getCurrentInstance();
    if (fc != null)
      l = FacesContext.getCurrentInstance().getExternalContext().getRequestLocale();
    return l;
  }

  public Type getType() {
    return type;
  }

  public enum Type {
    CHANGE, ADD, REMOVE
  }
}
