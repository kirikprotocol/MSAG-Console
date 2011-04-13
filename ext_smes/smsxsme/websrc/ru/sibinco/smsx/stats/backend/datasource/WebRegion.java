package ru.sibinco.smsx.stats.backend.datasource;

/**
 * @author Aleksandr Khalitov
 */
public class WebRegion {

  private final String region;

  private final boolean msc;

  private final int srcCount;

  private final int dstCount;

  public WebRegion(String region, boolean msc, int srcCount, int dstCount) {
    this.region = region;
    this.msc = msc;
    this.srcCount = srcCount;
    this.dstCount = dstCount;
  }

  public String getRegion() {
    return region;
  }

  public boolean isMsc() {
    return msc;
  }

  public int getSrcCount() {
    return srcCount;
  }

  public int getDstCount() {
    return dstCount;
  }

  public boolean equals(Object o) {
    if (this == o) return true;
    if (!(o instanceof WebRegion)) return false;

    WebRegion webRegion = (WebRegion) o;

    if (dstCount != webRegion.dstCount) return false;
    if (msc != webRegion.msc) return false;
    if (srcCount != webRegion.srcCount) return false;
    if (region != null ? !region.equals(webRegion.region) : webRegion.region != null) return false;

    return true;
  }

  public int hashCode() {
    int result = region != null ? region.hashCode() : 0;
    result = 31 * result + (msc ? 1 : 0);
    result = 31 * result + srcCount;
    result = 31 * result + dstCount;
    return result;
  }
}
