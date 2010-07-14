package ru.novosoft.smsc.admin.config;

import ru.novosoft.smsc.admin.AdminException;

/**
 * Интерфейс описывает конфигурацию, изменения в которой вступают в силу только после
 * вызова метода apply().
 * @author Artem Snopkov
 */
public interface AppliableConfiguration {

  /**
   * Применяет изменения, сделанные в конфигурации.
   * После применения изменений метод isChanged() вовращает false.
   * Если по каким-либо причинам исполнение метода apply() закончилось ошибкой,
   * конфигурация остается в её текущем состоянии и метод isChanged() возвращает true.
   * @throws AdminException если применить изменения невозможно.
   */
  void apply() throws AdminException;

  /**
   * Откатывает конфигурацию к последнему примененному состоянию.
   * После отката изменений метод isChanged() вовращает false.
   * Если по каким-либо причинам при исполнении метода reset() произошла ошибка,
   * конфигурация остается в её текущем состоянии и метод isChanged() возвращает true.
   * @throws AdminException если откатить изменния невозможно
   */
  void reset() throws AdminException;

  /**
   * Возвращает true, если в конфигурации произошли изменения
   * @return true, если в конфигурации произошли изменения
   */
  boolean isChanged();
}
