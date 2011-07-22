package mobi.eyeline.util.jsf.components.sample.controllers;

import java.text.SimpleDateFormat;
import java.util.Date;

/**
 * author: Aleksandr Khalitov
 */
public class UpdatableContentController {

  private int updatePeriod = 1;

  public int getUpdatePeriod() {
    return updatePeriod;
  }

  public void setUpdatePeriod(int updatePeriod) {
    this.updatePeriod = updatePeriod;
  }

  public String getDate() {
    return new SimpleDateFormat("yyyy.MM.dd HH:mm:ss").format(new Date());
  }
}
