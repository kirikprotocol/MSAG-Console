package mobi.eyeline.informer.web.controllers;

/**
 * @author Aleksandr Khalitov
 */
public class Index1Controller {

  private int current=0;

  public int getMaximum() {
    return 100;
  }

  public int getCurrent() {
    if(current < 100) {
      current++;
    }
    return current;
  }
}
