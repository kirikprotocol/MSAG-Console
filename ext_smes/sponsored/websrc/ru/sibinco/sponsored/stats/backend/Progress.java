package ru.sibinco.sponsored.stats.backend;

//todo Метод getProgress у данного класса не используется, а метод setProgress переопределяется.
//todo Думаю, правильнее будет заменить данный класс интерфейсом ProgressListener с одним методом setProgress(int progress)
//todo И этот интерфейс надо перенести в пакет datasource, чтобы устранить циклическую зависимость пакетов backend<->datasource. По идее должно быть так: bacnend->datasource
public class Progress {

  private int progress;

  public int getProgress() {
    return progress;
  }

  public void setProgress(int progress) {
    this.progress = progress;
  }
}