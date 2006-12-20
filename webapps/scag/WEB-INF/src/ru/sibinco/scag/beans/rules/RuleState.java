package ru.sibinco.scag.beans.rules;

import java.io.Serializable;
import java.util.HashMap;

/**
 * Created by IntelliJ IDEA.
 * User: dym
 * Date: 13.07.2006
 * Time: 13:29:01
 * To change this template use File | Settings | File Templates.
 */
public class RuleState implements Serializable {
  public static final String existError = "jEdit.rule_state.existError";
  public static final String notExistError = "jEdit.rule_state.notExistError";
  public static final String lockedError = "jEdit.rule_state.lockedError";

  private boolean exists;
  private boolean locked;

  public void setExists(boolean exists) {
    this.exists = exists;
  }
  public boolean getExists() {
    return exists;
  }
  public void setLocked(boolean locked) {
    this.locked = locked;
  }
  public boolean getLocked() {
    return locked;
  }

  public RuleState copy() {
    RuleState r = new RuleState();
    r.setExists(exists);
    r.setLocked(locked);
    return r;
  }

  public String toString() {
    return "exists = " + exists +
           " locked = " + locked;
  }
}
