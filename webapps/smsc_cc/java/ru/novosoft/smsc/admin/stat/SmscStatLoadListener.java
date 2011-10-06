package ru.novosoft.smsc.admin.stat;

/**
 * Листенер, предоставляющий информацию о ходе выполнения загрузки
 * author: Aleksandr Khalitov
 */
public interface SmscStatLoadListener {

  public void incrementProgress();

  public void setTotal(int numb);

}
