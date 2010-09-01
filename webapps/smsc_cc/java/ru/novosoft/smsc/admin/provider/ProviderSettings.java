package ru.novosoft.smsc.admin.provider;

import ru.novosoft.smsc.admin.AdminException;

import java.io.Serializable;
import java.util.*;

/**
 * Настройки провайдеров
 * @author Artem Snopkov
 */
public class ProviderSettings implements Serializable {

  private final Map<Long, Provider> providers;
  private long lastProviderId;

  ProviderSettings(Collection<Provider> providers, long lastProviderId) {
    this.providers = new HashMap<Long, Provider>();
    this.lastProviderId  = lastProviderId;
    for (Provider p : providers)
      this.providers.put(p.getId(), p);
  }

  ProviderSettings(ProviderSettings settings) {
    this(settings.getProviders(), settings.getLastProviderId());
  }

  private void checkName(String name, Long currentProviderId) throws AdminException {
    for (Provider p : providers.values())
      if (p.getName().equals(name) && (currentProviderId == null || currentProviderId != p.getId()))
        throw new ProviderException("provider.name.is.not.unique", name);
  }

  long getLastProviderId() {
    return lastProviderId;
  }

  /**
   * Добавляет в настройки и возвращает нового провайдера
   * @param name имя нового провайдера
   * @return инстанц Provider
   * @throws AdminException если имя провайдера некорректно или неуникально
   */
  public Provider addProvider(String name) throws AdminException {
    checkName(name, null);
    Provider newProvider = new Provider(++lastProviderId, name);
    providers.put(newProvider.getId(), newProvider);
    return new Provider(newProvider);
  }

  /**
   * Обновляет данные о провайдере
   * @param updatedProvider новые данные о провайдере
   * @throws AdminException если новое имя провайдера не уникально
   */
  public void updateProvider(Provider updatedProvider) throws AdminException {
    checkName(updatedProvider.getName(), updatedProvider.getId());
    providers.put(updatedProvider.getId(), new Provider(updatedProvider));
  }

  /**
   * Возвращает провайдера по его идентификатору
   * @param id идентификатор провайдера
   * @return  провайдера по его идентификатору или null, если провайдера нет
   */
  public Provider getProvider(long id) {
    Provider p = providers.get(id);
    if (p == null)
      return p;
    return new Provider(p);
  }

  /**
   * Возвращает список провайдеров
   * @return список провайдеров
   */
  public Collection<Provider> getProviders() {
    ArrayList<Provider> result = new ArrayList<Provider>(providers.size());
    for (Provider p : providers.values())
      result.add(new Provider(p));
    return result;
  }

  /**
   * Возвращает копию текущего инстанца ProviderSettings
   * @return копию текущего инстанца ProviderSettings
   */
  public ProviderSettings cloneSettings() {
    return new ProviderSettings(this);
  }
}
