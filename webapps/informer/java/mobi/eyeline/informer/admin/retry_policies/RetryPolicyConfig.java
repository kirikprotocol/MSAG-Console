package mobi.eyeline.informer.admin.retry_policies;


import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.util.config.ManagedConfigFile;
import mobi.eyeline.informer.util.config.XmlConfig;
import mobi.eyeline.informer.util.config.XmlConfigException;
import mobi.eyeline.informer.util.config.XmlConfigParam;
import mobi.eyeline.informer.util.config.XmlConfigSection;

import java.io.InputStream;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.List;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 11.10.2010
 * Time: 14:13:57
 */

class RetryPolicyConfig implements ManagedConfigFile<RetryPolicySettings> {

  public RetryPolicySettings load(InputStream is) throws Exception {

    RetryPolicySettings result = new RetryPolicySettings();
    XmlConfig config = new XmlConfig();
    config.load(is);
    XmlConfigSection  retryPolicies = config.getSection("RetryPolicies");
    for(XmlConfigSection s : retryPolicies.sections()) {
      RetryPolicy rp = createRetryPolicy(s);
      result.addRetryPolicy(rp);
    }
    return result;
  }

  private RetryPolicy createRetryPolicy(XmlConfigSection s) throws XmlConfigException, AdminException {
    int defaultTime = s.getParam("default").getInt();
    RetryPolicy rp = new RetryPolicy(s.getName(),defaultTime);
    List<RetryPolicyEntry> entries = new ArrayList<RetryPolicyEntry>();
    for(XmlConfigParam param : s.params()) {
      String name = param.getName();
      if(!"default".equals(name)) {
        int time = param.getInt();
        entries.add(new RetryPolicyEntry(Integer.valueOf(name),time));
      }
    }
    rp.setEntries(entries);
    return rp;
  }

  public void save(InputStream oldFile, OutputStream newFile, RetryPolicySettings conf) throws Exception {
    XmlConfig config = new XmlConfig();
    config.load(oldFile);


    XmlConfigSection  retryPolicies = new XmlConfigSection("RetryPolicies");

    for(RetryPolicy rpolicy : conf.getPolicies() ) {
      XmlConfigSection section = createSection(rpolicy);
      retryPolicies.addSection(section);
    }

    config.addSection(retryPolicies);
    config.save(newFile);
  }

  private XmlConfigSection createSection(RetryPolicy rp) {
    XmlConfigSection section = new XmlConfigSection(rp.getPolicyId());
    section.setInt("default",rp.getDefaultTime());
    for(RetryPolicyEntry rpe : rp.getEntries()) {
      section.setInt(String.valueOf(rpe.getErrCode()),rp.getDefaultTime());
    }
    return section;
  }

}