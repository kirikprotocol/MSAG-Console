package ru.novosoft.smsc.admin.archive_daemon;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.config.ManagedConfigFile;
import ru.novosoft.smsc.util.config.XmlConfig;
import ru.novosoft.smsc.util.config.XmlConfigException;
import ru.novosoft.smsc.util.config.XmlConfigParam;
import ru.novosoft.smsc.util.config.XmlConfigSection;

import java.io.InputStream;
import java.io.OutputStream;
import java.util.HashMap;
import java.util.Map;

/**
 * Класс для работы с конфигурационным файлом ArchiveDemon
 *
 * @author Aleksandr Khalitov
 */

@SuppressWarnings({"EmptyCatchBlock"})
class ArchiveDaemonConfig implements ManagedConfigFile {

  private ArchiveDaemonSettings settings;

  protected void reset(XmlConfig c) throws XmlConfigException, AdminException {
    ArchiveDaemonSettings s = new ArchiveDaemonSettings();
    XmlConfigSection config = c.getSection("ArchiveDaemon");

    s.setInterval(config.getInt("interval"));

    s.setViewHost(config.getSection("View").getString("host"));
    s.setViewPort(config.getSection("View").getInt("port"));
    s.setViewTimeout(config.getSection("View").getInt("timeout"));

    s.setQueriesMax(config.getSection("Queries").getInt("max"));
    s.setQueriesInit(config.getSection("Queries").getInt("init"));

    s.setTransactionsMaxSmsCount(config.getSection("Transactions").getInt("maxSmsCount"));
    s.setTransactionsMaxTimeInterval(config.getSection("Transactions").getInt("maxTimeInterval"));

    s.setLocationsBaseDestination(config.getSection("Locations").getString("baseDestination"));
    s.setLocationsTextDestinations(config.getSection("Locations").getString("textDestination"));

    Map<String, String> locationsSources = new HashMap<String, String>();
    XmlConfigSection sourcesSection = config.getSection("Locations").getSection("sources");
    for (XmlConfigParam sourceParam : sourcesSection.params())
      locationsSources.put(sourceParam.getName(), sourceParam.getString());

    s.setLocationsSources(locationsSources);

    s.setIndexatorMaxFlushSpeed(config.getSection("Indexator").getInt("maxFlushSpeed"));


    Map<String, Integer> indexatorSmeAddrChunkSizes = new HashMap<String, Integer>();
    XmlConfigSection chunkSizesSection = config.getSection("Indexator").getSection("smeAddrChunkSize");
    for (XmlConfigParam chunkSizeParam : chunkSizesSection.params())
      indexatorSmeAddrChunkSizes.put(chunkSizeParam.getName(), chunkSizeParam.getInt());

    s.setIndexatorSmeAddrChunkSizes(indexatorSmeAddrChunkSizes);

    this.settings = s;
  }

  public void load(InputStream is) throws Exception {
    XmlConfig config = new XmlConfig(is);
    reset(config);
  }

  protected void save(XmlConfig c) throws XmlConfigException {
    XmlConfigSection config = c.getSection("ArchiveDaemon");
    config.setInt("interval", settings.getInterval());

    config.getSection("View").setString("host", settings.getViewHost());
    config.getSection("View").setInt("port", settings.getViewPort());
    config.getSection("View").setInt("timeout", settings.getViewTimeout());

    config.getSection("Queries").setInt("max", settings.getQueriesMax());
    config.getSection("Queries").setInt("init", settings.getQueriesInit());

    config.getSection("Transactions").setInt("maxSmsCount", settings.getTransactionsMaxSmsCount());
    config.getSection("Transactions").setInt("maxTimeInterval", settings.getTransactionsMaxTimeInterval());

    config.getSection("Locations").setString("baseDestination", settings.getLocationsBaseDestination());
    config.getSection("Locations").setString("textDestination", settings.getLocationsTextDestinations());

    config.getSection("Locations").addSection("sources");
    XmlConfigSection sourcesSection = config.getSection("Locations").getSection("sources");
    sourcesSection.clear();
    for (Map.Entry<String, String> location : settings.getLocationsSources().entrySet())
      sourcesSection.setString(location.getKey(), location.getValue());

    config.getSection("Indexator").setInt("maxFlushSpeed", settings.getIndexatorMaxFlushSpeed());

    XmlConfigSection chunkSizesSection = config.getSection("Indexator").getSection("smeAddrChunkSize");
    chunkSizesSection.clear();
    for (Map.Entry<String, Integer> size : settings.getIndexatorSmeAddrChunkSizes().entrySet())
      chunkSizesSection.setInt(size.getKey(), size.getValue());
  }

  public void save(InputStream is, OutputStream os) throws Exception {
    XmlConfig c = new XmlConfig(is);
    save(c);
    c.save(os);
  }

  public ArchiveDaemonSettings getSettings() {
    return settings;
  }

  public void setSettings(ArchiveDaemonSettings settings) {
    this.settings = settings;
  }
}
