package ru.sibinco.smsx.stats;

import org.apache.log4j.Category;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.preferences.UserPreferences;
import ru.novosoft.smsc.jsp.SMEAppContext;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.sibinco.smsx.stats.backend.StatRequestManager;
import ru.sibinco.smsx.stats.backend.StatisticsException;

import java.io.File;
import java.io.IOException;
import java.security.Principal;
import java.util.Collection;
import java.util.Locale;

/**
 * @author Aleksandr Khalitov
 */
public class SmsxContext  implements SMEAppContext {

  private static final Category logger = Category.getInstance(SmsxContext.class);

  private static SmsxContext instance;

  private StatRequestManager statRequestManager;

  private SMSCAppContext smscAppContext;

  private File workDir;

  public static void init(SMSCAppContext smscAppContext) throws AdminException{
    instance = new SmsxContext(smscAppContext);
    smscAppContext.registerSMEContext(instance);
  }

  public static SmsxContext getInstance() {
    return instance;
  }

  private SmsxContext(SMSCAppContext smscAppContext) throws AdminException {
    this.smscAppContext = smscAppContext;
    try{
      workDir =  new File(smscAppContext.getConfig().getString("system.work folder"), "smsx");
      if(!workDir.exists() && !workDir.mkdirs()) {
        throw new StatisticsException("Can't create work dir: "+workDir.getAbsolutePath());
      }
      File smsxArtDir = new File(smscAppContext.getConfig().getString("smsx.stats.smsxArtefactsDir"));
      if(!smsxArtDir.exists()) {
        throw new StatisticsException("Artefacts dir doesn't exist: "+smsxArtDir.getAbsolutePath());
      }
      statRequestManager = new StatRequestManager(workDir, smsxArtDir);
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
