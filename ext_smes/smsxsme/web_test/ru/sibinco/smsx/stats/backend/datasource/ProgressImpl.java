package ru.sibinco.smsx.stats.backend.datasource;

/**
* @author Aleksandr Khalitov
*/
class ProgressImpl implements ProgressListener {
  private int progress;
  public void setProgress(int progress) {
    this.progress = progress;
  }
  public int getProgress() {
    return progress;
  }
}
