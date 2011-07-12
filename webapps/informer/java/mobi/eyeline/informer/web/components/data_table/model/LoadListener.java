package mobi.eyeline.informer.web.components.data_table.model;

import mobi.eyeline.informer.web.components.data_table.model.ModelException;

/**
* author: Aleksandr Khalitov
*/
public class LoadListener {

  private int current;
  private int total = Integer.MAX_VALUE;

  private ModelException loadError;

  private boolean finished;

  public int getCurrent() {
    return current;
  }

  public void setCurrent(int current) {
    this.current = current;
  }

  public int getTotal() {
    return total;
  }

  public void setTotal(int total) {
    this.total = total;
  }

  public ModelException getLoadError() {
    return loadError;
  }

  public void setLoadError(ModelException loadError) {
    this.loadError = loadError;
  }

  public boolean isFinished() {
    return finished;
  }

  public void setFinished(boolean finished) {
    this.finished = finished;
  }
}
