package ru.sibinco.smsx.stats.backend.datasource;

/**
 * @author Aleksandr Khalitov
 */
public class WebDaily {

  private final String date;

  private final String region;

  private final boolean msc;

  private final int count;

  public WebDaily(String date, String region, boolean msc, int count) {
    this.date = date;
    this.region = region;
    this.msc = msc;
    this.count = count;
  }

  public String getDate() {
    return date;
  }

  public String getRegion() {
    return region;
  }

  public boolean isMsc() {
    return msc;
  }

  public int getCount() {
    return count;
  }

  public boolean equals(Object o) {
    if (this == o) return true;
    if (!(o instanceof WebDaily)) return false;

    WebDaily webDaily = (WebDaily) o;

    if (count != webDaily.count) return false;
    if (msc != webDaily.msc) return false;
    if (date != null ? !date.equals(webDaily.date) : webDaily.date != null) return false;
    if (region != null ? !region.equals(webDaily.region) : webDaily.region != null) return false;

    return true;
  }

  public int hashCode() {
    int result = date != null ? date.hashCode() : 0;
    result = 31 * result + (region != null ? region.hashCode() : 0);
    result = 31 * result + (msc ? 1 : 0);
    result = 31 * result + count;
    return result;
  }
}
