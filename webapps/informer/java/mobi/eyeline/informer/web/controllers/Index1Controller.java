package mobi.eyeline.informer.web.controllers;

/**
 * @author Aleksandr Khalitov
 */
public class Index1Controller {

  private static int current=0;

  public int getMaximum() {
    return 100;
  }

  public boolean isContinueUpdate() {
    return current < 10;
  }


  public int getCurrent() {
    if(current < 1000) {
      current++;
    }
    return current;
  }
}
