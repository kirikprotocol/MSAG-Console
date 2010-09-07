package ru.novosoft.smsc.admin.profile;

/**
 * Структура, описывающая результаты поиска профиля
 * @author Artem Snopkov
 */
public class ProfileLookupResult {

  private final Profile profile;
  private final boolean exactMatch;
  private final boolean maskMatch;
  private final boolean defaultMatch;

  ProfileLookupResult(Profile profile, boolean exactMatch, boolean maskMatch, boolean defaultMatch) {
    this.profile = profile;
    this.exactMatch = exactMatch;
    this.maskMatch = maskMatch;
    this.defaultMatch = defaultMatch;
  }

  /**
   * Возвращает найденный профиль. Данное значение никогда не бывает null.
   * @return найденный профиль
   */
  public Profile getProfile() {
    return profile;
  }

  /**
   * Возвращает true, если найден точный профиль
   * @return true, если найден точный профиль
   */
  public boolean isExactMatch() {
    return exactMatch;
  }

  /**
   * Возвращает true, если найден групповой профиль
   * @return true, если найден групповой профиль
   */
  public boolean isMaskMatch() {
    return maskMatch;
  }

  /**
   * Возвращает true, если найден дефолтный профиль
   * @return true, если найден дефолтный профиль 
   */
  public boolean isDefaultMatch() {
    return defaultMatch;
  }
}
