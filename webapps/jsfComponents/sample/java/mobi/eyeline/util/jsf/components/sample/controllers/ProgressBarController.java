package mobi.eyeline.util.jsf.components.sample.controllers;

import java.util.Random;

/**
 * author: Aleksandr Khalitov
 */
public class ProgressBarController {

  private boolean disabled;

  public int getNextInt() {
    return  new Random().nextInt(100);
  }

  public String enable() {
    disabled = false;
    return null;
  }

  public String disable() {
    disabled = true;
    return null;
  }

  public boolean isEnabled() {
    return !disabled;
  }



}
