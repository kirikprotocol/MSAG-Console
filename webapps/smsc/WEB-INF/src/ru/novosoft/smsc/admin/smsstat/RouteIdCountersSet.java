package ru.novosoft.smsc.admin.smsstat;

/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 10, 2002
 * Time: 4:38:25 PM
 * To change this template use Options | File Templates.
 */

import ru.novosoft.smsc.admin.category.Category;
import ru.novosoft.smsc.admin.provider.Provider;

public class RouteIdCountersSet extends ExtendedCountersSet implements Comparable
{
  public String routeid;
  public long providerId = -1;
  public long categoryId = -1;
  public String providerName = "";
  public String categoryName = "";
  public Provider provider = new Provider(-1, "");
  public Category category = new Category(-1, "");

  public RouteIdCountersSet(String routeid)
  {
    this.routeid = routeid;
  }

  public RouteIdCountersSet(long accepted, long rejected, long delivered,
                            long failed, long rescheduled, long temporal, long i, long o, String routeid)
  {
    super(accepted, rejected, delivered, failed, rescheduled, temporal, i, o);
    this.routeid = routeid;
  }

  public RouteIdCountersSet(long accepted, long rejected, long delivered, long failed, long rescheduled,
                            long temporal, long i, long o, String routeid, Provider provider, Category category)
  {
    super(accepted, rejected, delivered, failed, rescheduled, temporal, i, o);
    this.routeid = routeid;
    this.provider = provider;
    this.category = category;
    this.providerName = provider.getName();
    this.categoryName = category.getName();
    this.providerId = provider.getId();
    this.categoryId = category.getId();
  }

  public int compareTo(Object o)
  {
    if (o == null || routeid == null || !(o instanceof RouteIdCountersSet)) return -1;
    return routeid.compareTo(((RouteIdCountersSet) o).routeid);
  }

    public void setProvider(Provider provider) {
        this.provider = provider;
    }

    public void setCategory(Category category) {
        this.category = category;
    }
}
