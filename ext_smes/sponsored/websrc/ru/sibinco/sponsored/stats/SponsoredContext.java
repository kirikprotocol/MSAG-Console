package ru.sibinco.sponsored.stats;

import org.apache.log4j.Category;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.preferences.UserPreferences;
import ru.novosoft.smsc.jsp.SMEAppContext;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.util.config.Config;
import ru.sibinco.sponsored.stats.backend.StatRequestManager;
import ru.sibinco.sponsored.stats.backend.StatisticsException;

import java.io.File;
import java.io.IOException;
import java.security.Principal;
import java.util.Collection;
import java.util.Locale;

/**
 * @author Aleksandr Khalitov
 */
public class SponsoredContext implements SMEAppContext {

  private static final Category logger = Category.getInstance(SponsoredContext.class);

  private static SponsoredContext instance;

  private StatRequestManager statRequestManager;

  private SMSCAppContext smscAppContext;

  private File workDir;

  public static void init(SMSCAppContext smscAppContext) throws AdminException{
    instance = new SponsoredContext(smscAppContext);
    smscAppContext.registerSMEContext(instance);
  }

  public static SponsoredContext getInstance() {
    return instance;
  }

  private SponsoredContext(SMSCAppContext smscAppContext) throws AdminException {
    this.smscAppContext = smscAppContext;
    try{
      workDir =  new File(smscAppContext.getConfig().getString("system.work folder"), "sponsored");
      if(!workDir.exists() && !workDir.mkdirs()) {
        throw new StatisticsException("Can't create work dir: "+workDir.getAbsolutePath());
      }

      File sponsoredHome = smscAppContext.getHostsManager().getServiceInfo("sponsored").
          getServiceFolder();
      System.out.println("sponsored home: "+sponsoredHome.getAbsolutePath());
      Config sponsoredConfig = new Config(new File(sponsoredHome, "conf"+File.separatorChar+"config.xml"));
      File sponsArtDir = new File(sponsoredConfig.getString("file_storage.storeDir"));
      System.out.println("sponsored artefacts: "+sponsArtDir.getAbsolutePath());
      if(!sponsArtDir.exists()) {
        throw new StatisticsException("Artefacts dir doesn't exist: "+sponsArtDir.getAbsolutePath());
      }
      statRequestManager = new StatRequestManager(workDir,sponsArtDir);
    }catch (Exception e){
      logger.error(e,e);
      throw new AdminException(e.getMessage());
    }
  }

  public StatRequestManager getStatRequestManager() {
    return statRequestManager;
  }

  public Locale getUserLocale(Principal principal) {
    UserPreferences uP = smscAppContext.getUserManager().getPrefs(principal);
    return uP != null ? uP.getLocale() : Locale.ENGLISH;
  }

  public Collection getRegions() {
    return smscAppContext.getRegionsManager().getRegions();
  }

  public File getWorkDir() {
    return workDir;
  }

  public void shutdown() throws IOException {
    if(statRequestManager != null) {
      statRequestManager.shutdown();
    }
  }
}
