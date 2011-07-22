package mobi.eyeline.util.jsf.components.sample.controllers;

import java.util.Random;

/**
 * author: Aleksandr Khalitov
 */
public class ProgressBarController {

  private int nextInt = new Random().nextInt(100);

  public int getNextInt() {
    return nextInt;
  }

}
