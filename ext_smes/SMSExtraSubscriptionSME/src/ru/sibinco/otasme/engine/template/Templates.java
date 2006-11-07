package ru.sibinco.otasme.engine.template;

import org.apache.log4j.Category;
import ru.sibinco.otasme.engine.template.parser.TemplatesReader;
import ru.sibinco.otasme.utils.Service;
import ru.sibinco.otasme.utils.Utils;

import java.io.File;
import java.util.ArrayList;
import java.util.Properties;

/**
 * User: artem
 * Date: 25.10.2006
 */

public final class Templates {

  private static final Category log = Category.getInstance(Templates.class);
  private static final int RELOAD_INTERVAL;

  private static Templates instance;
  private static final Object instanceLock = new Object();

  static {
    final Properties config = Utils.loadConfig("sme.properties");
    RELOAD_INTERVAL = Utils.loadInt(config, "templates.reload.period");

    try {
      instance = TemplatesReader.readTemplates("config/templates.xml");

      new TemplatesReloader().startService();
    } catch (TemplatesReader.TemplatesReaderException e) {
      log.error("Error reading templates!!!", e);
      instance = null;
    }
  }

  public static MacroRegion getMacroRegionByAbonentNumber(String number) {
    synchronized (instanceLock) {
      return instance.getMacroRegionByAbonentNumberInternal(number);
    }
  }


  private ArrayList macroRegions = new ArrayList();

  public void addMacroRegion(MacroRegion region) {
    macroRegions.add(region);
  }

  private MacroRegion getMacroRegionByAbonentNumberInternal(String number) {
    if (number.startsWith("+7"))
      number = number.substring(2);
    for (int i=0; i<macroRegions.size(); i++) {
      final MacroRegion region = (MacroRegion)macroRegions.get(i);
      if (region.containsNumber(number)) {
        log.error("Abonent = " + number + ", region = " + region.getName() + ", SMSC number = " + region.getNumber());
        return region;
      }
    }

    log.error("WARNING!!! Macro region not found for abonent " + number);
    return null;
  }

  private static class TemplatesReloader extends Service {
    private boolean errorOccured = false;
    private long lastModifiedTime = -1;

    TemplatesReloader() {
      super(log);
    }

    public synchronized void iterativeWork() {
      try {
        wait((!errorOccured) ? RELOAD_INTERVAL : 15000);
      } catch (InterruptedException e) {
        log.error("Interrupted", e);
      }

      final File f = new File("config/templates.xml");

      if (lastModifiedTime != f.lastModified()) {
        try {
          final Templates templates = TemplatesReader.readTemplates("config/templates.xml");

          if (templates != null) {
            log.info("Templates has been reloaded");
            errorOccured = false;
            lastModifiedTime = f.lastModified();

            synchronized(instanceLock) {
              instance = templates;
            }
          } else
            log.info("Can't read templates. Error.");

        } catch (TemplatesReader.TemplatesReaderException e) {
          log.error("Error reading templates!!!", e);
          errorOccured = true;
        }
      }
    }
  }


}
