package mobi.eyeline.informer.admin.archive;

import mobi.eyeline.informer.util.config.XmlConfigException;
import mobi.eyeline.informer.util.config.XmlConfigSection;

/**
 * @author Aleksandr Khalitov
 */
public class ArchiveSettings {

  private String resultsDir;

  private String requestsDir;

  private int chunkSize;

  private int executorsSize;

  public ArchiveSettings(ArchiveSettings settings) {
    this.resultsDir = settings.resultsDir;
    this.requestsDir = settings.requestsDir;
    this.chunkSize = settings.chunkSize;
    this.executorsSize = settings.executorsSize;
  }

  public ArchiveSettings() {
  }

  public String getResultsDir() {
    return resultsDir;
  }

  public void setResultsDir(String resultsDir) {
    this.resultsDir = resultsDir;
  }

  public String getRequestsDir() {
    return requestsDir;
  }

  public void setRequestsDir(String requestsDir) {
    this.requestsDir = requestsDir;
  }

  public int getChunkSize() {
    return chunkSize;
  }

  public void setChunkSize(int chunkSize) {
    this.chunkSize = chunkSize;
  }

  public int getExecutorsSize() {
    return executorsSize;
  }

  public void setExecutorsSize(int executorsSize) {
    this.executorsSize = executorsSize;
  }

  public void save(XmlConfigSection s) {
    if(resultsDir != null) {
      s.setString("resultsDir", resultsDir);
    }
    if(requestsDir != null) {
      s.setString("requestsDir", requestsDir);
    }
    s.setInt("chunkSize", chunkSize);
    s.setInt("executorsSize", executorsSize);
  }

  public void load(XmlConfigSection s) throws XmlConfigException {
    resultsDir = s.getString("resultsDir", null);
    requestsDir = s.getString("requestsDir", null);
    chunkSize = s.getInt("chunkSize", 0);
    executorsSize = s.getInt("executorsSize", 0);
  }
}
