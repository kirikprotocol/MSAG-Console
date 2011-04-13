package ru.sibinco.smsx.stats;

import org.apache.log4j.Category;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.preferences.UserPreferences;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.util.config.Config;
import ru.sibinco.smsx.stats.backend.StatRequestManager;
import ru.sibinco.smsx.stats.backend.StatisticsException;

import java.io.File;
import java.security.Principal;
import java.util.Collection;
import java.util.Locale;

/**
 * @author Aleksandr Khalitov
 */
public class SmsxContext {

  private static final Category logger = Category.getInstance(SmsxContext.class);

  private static SmsxContext instance;

  private StatRequestManager statRequestManager;

  private SMSCAppContext smscAppContext;

  private File workDir;

  public static void init(SMSCAppContext smscAppContext, Config config) throws AdminException{
    instance = new SmsxContext(config, smscAppContext);
  }

  public static SmsxContext getInstance() {
    return instance;
  }

  private SmsxContext(Config c, SMSCAppContext smscAppContext) throws AdminException {
    this.smscAppContext = smscAppContext;
    try{
      File reqDir =  new File(c.getString("smsx.stats.requestsDir"));
      if(!reqDir.exists() && !reqDir.mkdirs()) {
        throw new StatisticsException("Can't create requests dir: "+reqDir.getAbsolutePath());
      }
      workDir = reqDir;
      File smsxArtDir = new File(c.getString("smsx.stats.smsxArtefactsDir"));
      if(!smsxArtDir.exists()) {
        throw new StatisticsException("Artefacts dir doesn't exist: "+smsxArtDir.getAbsolutePath());
      }
      File sponsArtDir = new File(c.getString("smsx.stats.sponsoredArtefactsDir"));
      if(!sponsArtDir.exists()) {
        throw new StatisticsException("Artefacts dir doesn't exist: "+sponsArtDir.getAbsolutePath());
      }
      statRequestManager = new StatRequestManager(reqDir, smsxArtDir);
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
}
