package ru.novosoft.smsc.dbsme;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.util.Functions;
import ru.novosoft.smsc.util.config.Config;

import javax.servlet.http.HttpServletRequest;
import javax.xml.parsers.ParserConfigurationException;
import java.io.File;
import java.io.IOException;
import java.util.*;

import org.xml.sax.SAXException;


/**
 * Created by igork
 * Date: Jul 10, 2003
 * Time: 6:02:35 PM
 */
public class Index extends DbsmeBean
{
  private String mbApply = null;
  private String mbReset = null;
  private String mbStart = null;
  private String mbStop = null;
  private String[] apply = new String[0];
  private Set applySet = new HashSet();

  protected int init(List errors)
  {
    int result = super.init(errors);
    if (result != RESULT_OK)
      return result;

    applySet.addAll(Arrays.asList(apply));
    return result;
  }

  public int process(HttpServletRequest request)
  {
    int result = super.process(request);
    if (result != RESULT_OK)
      return result;

    if (mbApply != null) return apply();
    if (mbReset != null) return reset();
    if (mbStart != null) return start();
    if (mbStop  != null) return stop();

    return result;
  }

  private int stop()
  {
    try {
      appContext.getHostsManager().shutdownService(getSmeId());
    } catch (AdminException e) {
      return error("dbsme.error.stop", e);
    }
    return RESULT_DONE;
  }

  private int start()
  {
    try {
      appContext.getHostsManager().startService(getSmeId());
    } catch (AdminException e) {
      return error("dbsme.error.start", e);
    }
    return RESULT_DONE;
  }

  private int reset()
  {
    try { restoreFromOriginalConfig(); } catch (Exception e) {
      logger.error("Failed to restore from original config", e);
      return error("dbsme.error.config_reset", e);
    }
    getContext().setConfigChanged(false);
    getContext().setJobsChanged(false);
    return RESULT_DONE;
  }

  private int apply()
  {
    if (isApplyAll()) return applyAll();
    else if (isApplyJobs()) {
      if (isOnline()) return applyJobs();
      else return applyAll();
    }

    return RESULT_OK;
  }

  private int applyJobs()
  {
    int result = RESULT_DONE;
    Config originalConfig = null;
    try { originalConfig = new Config(getOriginalConfigFile()); } catch (Throwable e) {
      logger.error("Could not get original config", e);
      return error("dbsme.error.no_config", e);
    }

    try { originalConfig.save(); } catch (Throwable e) {
      logger.error("Could not create backup copy of config", e);
      return error("dbsme.error.config_backup", e);
    }

    Set providers = config.getSectionChildShortSectionNames("DBSme.DataProviders");
    for (Iterator i = providers.iterator(); i.hasNext();) {
      String providerName = (String) i.next();
      int newResult = applyJobs(originalConfig, providerName);
      if (newResult != RESULT_DONE)
        result = newResult;
    }

    if (result == RESULT_DONE)
      getContext().setJobsChanged(false);

    return result;
  }

  private int applyJobs(Config originalConfig, String providerName)
  {
    int result = RESULT_DONE;

    final String providerPrefix = Provider.createProviderPrefix(providerName);
    Set jobIdsNew = config.getSectionChildShortSectionNames(providerPrefix + ".Jobs");
    Set jobIdsOld = originalConfig.getSectionChildShortSectionNames(providerPrefix + ".Jobs");
    Set removed = new HashSet(jobIdsOld);
    removed.removeAll(jobIdsNew);
    Set added = new HashSet(jobIdsNew);
    added.removeAll(jobIdsOld);
    Set changed = new HashSet(jobIdsNew);
    changed.removeAll(added);
    for (Iterator i = new ArrayList(changed).iterator(); i.hasNext();) {
      String jobId = (String) i.next();
      if (isJobsEquals(originalConfig, config, providerName, jobId))
        changed.remove(jobId);
    }

    try {

      for (Iterator i = removed.iterator(); i.hasNext();) {
        String jobId = (String) i.next();
        Config backup = (Config) originalConfig.clone();
        try {
          originalConfig.removeSection(Job.createJobPrefix(providerName, jobId));
          originalConfig.saveWithoutBackup();
          getSmeTransport().removeJob(jobId, providerName);
        } catch (Throwable e) {
          originalConfig = rollbackJob(jobId, e, backup, "remove", "removing");
          result = RESULT_ERROR;
        }
      }

      for (Iterator i = added.iterator(); i.hasNext();) {
        String jobId = (String) i.next();
        Config backup = (Config) originalConfig.clone();
        try {
          originalConfig.copySectionFromConfig(config, Job.createJobPrefix(providerName, jobId));
          originalConfig.saveWithoutBackup();
          getSmeTransport().addJob(jobId, providerName);
        } catch (Throwable e) {
          originalConfig = rollbackJob(jobId, e, backup, "add", "adding");
          result = RESULT_ERROR;
        }
      }

      for (Iterator i = changed.iterator(); i.hasNext();) {
        String jobId = (String) i.next();
        Config backup = (Config) originalConfig.clone();
        try {
          final String jobPrefix = Job.createJobPrefix(providerName, jobId);
          originalConfig.removeSection(jobPrefix);
          originalConfig.copySectionFromConfig(config, jobPrefix);
          originalConfig.saveWithoutBackup();
          getSmeTransport().changeJob(jobId, providerName);
        } catch (Throwable e) {
          originalConfig = rollbackJob(jobId, e, backup, "change", "changing");
          result = RESULT_ERROR;
        }
      }

    } catch (CloneNotSupportedException e) {
      logger.fatal("Inconsistent classes: " + Config.class.getName() + " does not implement java.lang.Cloneable", e);
      error("dbsme.error.internal", e);
    }

    return result;
  }

  private Config rollbackJob(String jobId, Throwable e, Config backup, String action, String actionContinue)
  {
    logger.error("Could not " + action + " job \"" + jobId + '"', e);
    //error("Could not " + action + " job", jobId, e);
    error("dbsme.error.failed_to", action + " job" + jobId, e);
    try {
      backup.save();
    } catch (Throwable e1) {
      logger.error("Could not rollback job \"" + jobId + "\" " + actionContinue, e1);
      //error("Could not rollback job " + actionContinue, jobId, e1);
      error("dbsme.error.rollback_job", actionContinue + " " + jobId, e1);
    }
    return backup;
  }

  private static boolean isJobsEquals(Config oldConfig, Config newConfig, String providerName, String jobId)
  {
    String jobPrefix = Job.createJobPrefix(providerName, jobId);
    try {
      return
          oldConfig.isParamEquals(newConfig, jobPrefix + ".name")
          && oldConfig.isParamEquals(newConfig, jobPrefix + ".type")
          && oldConfig.isParamEquals(newConfig, jobPrefix + ".sql")
          && oldConfig.isParamEquals(newConfig, jobPrefix + ".input")
          && oldConfig.isParamEquals(newConfig, jobPrefix + ".output")

          && oldConfig.isParamEquals(newConfig, jobPrefix + ".address")
          && oldConfig.isParamEquals(newConfig, jobPrefix + ".alias")
          && oldConfig.isParamEquals(newConfig, jobPrefix + ".timeout")
          && oldConfig.isParamEquals(newConfig, jobPrefix + ".query")
          && oldConfig.isParamEquals(newConfig, jobPrefix + ".commit")

          && oldConfig.isParamEquals(newConfig, jobPrefix + ".function")
          && oldConfig.isParamEquals(newConfig, jobPrefix + ".MessageSet.DS_FAILURE")
          && oldConfig.isParamEquals(newConfig, jobPrefix + ".MessageSet.DS_CONNECTION_LOST")
          && oldConfig.isParamEquals(newConfig, jobPrefix + ".MessageSet.DS_STATEMENT_FAIL")
          && oldConfig.isParamEquals(newConfig, jobPrefix + ".MessageSet.QUERY_NULL")
          && oldConfig.isParamEquals(newConfig, jobPrefix + ".MessageSet.INPUT_PARSE")
          && oldConfig.isParamEquals(newConfig, jobPrefix + ".MessageSet.OUTPUT_FORMAT")
          && oldConfig.isParamEquals(newConfig, jobPrefix + ".MessageSet.INVALID_CONFIG");
    } catch (Throwable e) {
      return false;
    }
  }

  private int applyAll()
  {
    final File origConfigFile = getOriginalConfigFile();

    try {
      Functions.renameNewSavedFileToOriginal(getTempConfigFile(), origConfigFile);
    } catch (IOException e) {
      logger.error("Could not rename current config file to original", e);
      return error("dbsme.error.config_replace", e);
    }

    getContext().setConfigChanged(false);
    getContext().setJobsChanged(false);
    if (isOnline()) {
      try {
        getSmeTransport().restart();
      } catch (AdminException e) {
        logger.error("Changes saved, but DBSme not restarted", e);
        return error("dbsme.error.apply_restart", e);
      }
    }
    return RESULT_DONE;
  }

  public boolean isConfigChanged()
  {
    return getContext().isConfigChanged();
  }

  public boolean isJobsChanged()
  {
    return getContext().isJobsChanged();
  }

  public boolean isApplyAll()
  {
    return applySet.contains("all");
  }

  public boolean isApplyJobs()
  {
    return applySet.contains("jobs");
  }

  public String getMbApply()
  {
    return mbApply;
  }

  public void setMbApply(String mbApply)
  {
    this.mbApply = mbApply;
  }

  public String getMbReset()
  {
    return mbReset;
  }

  public void setMbReset(String mbReset)
  {
    this.mbReset = mbReset;
  }

  public String getMbStart()
  {
    return mbStart;
  }

  public void setMbStart(String mbStart)
  {
    this.mbStart = mbStart;
  }

  public String getMbStop()
  {
    return mbStop;
  }

  public void setMbStop(String mbStop)
  {
    this.mbStop = mbStop;
  }

  public String[] getApply()
  {
    return apply;
  }

  public void setApply(String[] apply)
  {
    this.apply = apply;
  }
}
