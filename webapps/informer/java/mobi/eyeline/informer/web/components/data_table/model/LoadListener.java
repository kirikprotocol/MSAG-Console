package mobi.eyeline.informer.web.components.data_table.model;

/**
* author: Aleksandr Khalitov
*/
public class LoadListener{

  private int current;
  private int total = Integer.MAX_VALUE;

  private ModelException loadError;

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


}
