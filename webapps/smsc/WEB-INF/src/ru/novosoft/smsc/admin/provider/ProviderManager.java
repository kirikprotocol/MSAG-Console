package ru.novosoft.smsc.admin.provider;

import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.jsp.util.tables.DataSource;
import ru.novosoft.smsc.jsp.util.tables.impl.AbstractDataSourceImpl;
import ru.novosoft.smsc.jsp.util.tables.impl.provider.ProviderDataItem;
import ru.novosoft.smsc.util.StringEncoderDecoder;
import ru.novosoft.smsc.util.config.Config;

import java.io.IOException;
import java.util.*;


/**
 * Created by andrey Date: 07.02.2005 Time: 12:45:18
 */
public class ProviderManager extends AbstractDataSourceImpl implements DataSource
{
  private static final String[] COLUMN_NAMES = new String[]{"id", "name"};
  private final Map providers = Collections.synchronizedMap(new TreeMap());
  private int lastUsedId;
  private static final String SECTION_NAME_providers = "providers";
  private static final String PARAM_NAME_last_used_id = "last used provider id";
  protected SMSCAppContext appContext = null;
  private Config webappConfig = null;

  public ProviderManager(final Config gwConfig) throws Config.WrongParamTypeException, Config.ParamNotFoundException, NumberFormatException
  {
    super(COLUMN_NAMES);
    this.webappConfig = gwConfig;
    if (!gwConfig.containsSection(SECTION_NAME_providers)) {
      gwConfig.setInt(SECTION_NAME_providers + '.' + PARAM_NAME_last_used_id, 0);
    }
    lastUsedId = gwConfig.getInt(SECTION_NAME_providers + '.' + PARAM_NAME_last_used_id);
    final Collection providerIds = gwConfig.getSectionChildShortParamsNames(SECTION_NAME_providers);
    for (Iterator i = providerIds.iterator(); i.hasNext();) {
      final String providerIdStr = (String) i.next();
      if (!PARAM_NAME_last_used_id.equalsIgnoreCase(providerIdStr)) {
        final Long providerId = Long.decode(providerIdStr);
        //  final Provider provider = createProvider(providerId.longValue(), gwConfig.getString(SECTION_NAME_providers + '.' + providerIdStr));
        //  providers.put(providerId, provider);
        final Provider provider = new Provider(providerId.longValue(), gwConfig.getString(SECTION_NAME_providers + '.' + providerIdStr));
        addProvider(provider);
      }
    }
  }

/*
   public synchronized QueryResultSet query(Query query_to_run)
  {

    Vector sortOrder=query_to_run.getSortOrder();
     boolean isNegativeSort = false;
    String  sort = (String) sortOrder.get(0);
    
     if (sort.charAt(0) == '-') {
      sort = sort.substring(1);
      isNegativeSort = true;
    }

    QueryResultSetImpl result = new QueryResultSetImpl(Provider.columnNames, query_to_run.getSortOrder());

    List filteredProviders = filterProviders(query_to_run.getFilter());
    sortProviders(sort, isNegativeSort, filteredProviders);
    result.setLast(true);
    int lastpos = Math.min(query_to_run.getStartPosition() + query_to_run.getExpectedResultsQuantity(), filteredProviders.size());
    for (int i = query_to_run.getStartPosition(); i < lastpos; i++) {
      result.add((ProviderDataItem) filteredProviders.get(i));
      result.setLast(i == (filteredProviders.size() - 1));
    }
    result.setTotalSize(filteredProviders.size());

    return result;

  }

  private List filterProviders(Filter filter)
  {
    List result = new LinkedList();
    for (Iterator i = providers.values().iterator(); i.hasNext();) {
      Provider provider = (Provider) i.next();
      ProviderDataItem providerItem = new ProviderDataItem(provider);
      if (filter.isItemAllowed(providerItem))
        result.add(providerItem);
    }
    return result;
  }
   private void sortProviders(final String sort, final boolean isNegativeSort, List filteredProviders)
  {
    Collections.sort(filteredProviders, new Comparator()
    {
      public int compare(Object o1, Object o2)
      {
        if (o1 instanceof ProviderDataItem && o2 instanceof ProviderDataItem) {
          ProviderDataItem u1 = (ProviderDataItem) o1;
          ProviderDataItem u2 = (ProviderDataItem) o2;
          String ut1=(String) u1.getValue(sort);
          String ut2=(String) u2.getValue(sort);
          int result = ut1.compareToIgnoreCase(ut2);
          //int result = ((String) u1.getValue(sort)).compareToIgnoreCase((String) u2.getValue(sort));
          return isNegativeSort
                 ? -result
                 : result;
        } else
          return 0;
      }
    });
  }
*/

  public synchronized Map getProviders()
  {
    return providers;
  }

  public synchronized Provider getProvider(Long id)
  {
    //final Long providerId = Long.decode(id);
    return (Provider) providers.get(id);
  }

  public synchronized Provider getProviderByName(String name)
  {
    Provider providerFounded = null;
    for (Iterator iterator = providers.values().iterator(); iterator.hasNext();) {
      Provider provider = (Provider) iterator.next();
      if (provider.getName().equals(name)) {
        providerFounded = provider;
      }
    }
    return providerFounded;
  }


  public synchronized boolean addProvider(Provider provider)
  {
    if (!providers.containsKey(new Long(provider.getId()))) {
      providers.put(new Long(provider.getId()), provider);
      super.add(new ProviderDataItem(provider));
      return true;
    }
    else
      return false;
  }

  public synchronized Provider createProvider(final String name)
  {
    return new Provider(++lastUsedId, name);
  }

  public synchronized void setProviderName(final long id, final String name) throws NullPointerException
  {
    final Provider provider = (Provider) providers.get(new Long(id));
    if (null == provider)
      throw new NullPointerException("Provider \"" + id + "\" not found.");
    provider.setName(name);
  }

  public synchronized void apply()
          throws Exception
  {// webappConfig= appContext.getConfig();
    store(webappConfig);
    // XmlAuthenticator.init(configFile);
  }


  public synchronized void store(final Config gwConfig) throws Config.WrongParamTypeException, IOException
  {
    gwConfig.removeSection(SECTION_NAME_providers);
    gwConfig.setInt(SECTION_NAME_providers + '.' + PARAM_NAME_last_used_id, lastUsedId);
    for (Iterator i = providers.values().iterator(); i.hasNext();) {
      final Provider provider = (Provider) i.next();
      gwConfig.setString(SECTION_NAME_providers + "." + provider.getId(), StringEncoderDecoder.encode(provider.getName()));
    }
    gwConfig.save();
  }


  public Provider removeProvider(String name)
  {
    long id = 0;
    for (Iterator i = providers.values().iterator(); i.hasNext();) {
      Provider provider = (Provider) i.next();
      if (provider.getName().equals(name)) {
        id = provider.getId();
      }
    }
    final Long providerId = new Long(id);
    final Provider provider = (Provider) providers.remove(providerId);
    remove(new ProviderDataItem(provider));
    return provider;
  }
}
