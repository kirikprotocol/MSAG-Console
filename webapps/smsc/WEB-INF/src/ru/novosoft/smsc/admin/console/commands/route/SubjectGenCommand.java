/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 16, 2002
 * Time: 7:05:13 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console.commands.route;


import ru.novosoft.smsc.admin.console.Command;

import java.util.ArrayList;

public abstract class SubjectGenCommand implements Command
{
  protected String subject;
  protected String defaultSmeId = null;
  protected ArrayList masks = new ArrayList();
  protected String notes = "";
  protected boolean isNotes = false;

  public void setDefaultSmeId(String defaultSmeId) {
      this.defaultSmeId = defaultSmeId;
  }
  public void setSubject(String subject) {
      this.subject = subject;
  }
  public void setNotes(String notes) {
    this.notes = notes; isNotes = true;
  }
  public void addMask(String mask) {
      masks.add(mask);
  }
}

