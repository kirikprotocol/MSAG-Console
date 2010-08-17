package ru.sibinco.scag.backend.gw;

import ru.sibinco.scag.backend.Manager;
import ru.sibinco.scag.backend.SCAGAppContext;
import ru.sibinco.scag.beans.SCAGJspException;
import ru.sibinco.scag.Constants;
import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.StatusDisconnectedException;
import ru.sibinco.lib.backend.util.config.Config;
import org.apache.log4j.Logger;

import java.io.IOException;

/**
 * Created by IntelliJ IDEA.
 * User: dym
 * Date: 05.07.2006
 * Time: 12:58:51
 * To change this template use File | Settings | File Templates.
 */
public class ConfigManager extends Manager {
  private Logger logger = Logger.getLogger(this.getClass());
  private final String configFilename;
  private final Config gwConfig;
  public ConfigManager(final String configFilename, final Config gwConfig) {
    this.configFilename = configFilename;
    this.gwConfig = gwConfig;
  }

  public synchronized void applyConfig(final SCAGAppContext appContext, final Config oldConfig) throws SCAGJspException {
     try {
         appContext.getScag().invokeCommand("applyConfig", null, appContext, this, configFilename);
     } catch (SibincoException e) {
        if (!(e instanceof StatusDisconnectedException)) {
          //restore - copy parameters from oldConfig to current
          gwConfig.copyParams(oldConfig);
          logger.debug("Couldn't apply config", e);
          throw new SCAGJspException(Constants.errors.status.COULDNT_APPLY_CONFIG, e);
        }
     }
  }

  public void store() throws SibincoException{
      logger.debug( "ConfigManager.store()");
    try {
     gwConfig.saveWithoutBackup(System.getProperty("file.encoding"));
    } catch (Config.WrongParamTypeException e) {
        logger.debug("Couldn't save config", e);
        throw new SCAGJspException(Constants.errors.status.COULDNT_SAVE_CONFIG, e);
    } catch (IOException e) {
        logger.debug("Couldn't save config", e);
        throw new SCAGJspException(Constants.errors.status.COULDNT_SAVE_CONFIG, e);
    }
  }
}
