package mobi.eyeline.informer.admin.smsc;

import mobi.eyeline.informer.admin.util.config.ManagedConfigFile;
import mobi.eyeline.informer.util.config.XmlConfig;
import mobi.eyeline.informer.util.config.XmlConfigSection;

import java.io.InputStream;
import java.io.OutputStream;
import java.util.Collection;
import java.util.LinkedList;

/**
 * @author Aleksandr Khalitov
 */
class SmscConfig implements ManagedConfigFile<SmscSettings> {

  public void save(InputStream oldFile, OutputStream newFile, SmscSettings conf) throws Exception {
    XmlConfig config = new XmlConfig();
    config.load(oldFile);
    XmlConfigSection  smscConnectors = config.getOrCreateSection("SMSCConnectors");
    smscConnectors.clearSections();
    for(Smsc s : conf.getSmscs()) {
      XmlConfigSection section = new XmlConfigSection(s.getName());
      s.save(section);
      smscConnectors.addSection(section);
    }

    config.addSection(smscConnectors);
    smscConnectors.setString("default", conf.getDefaultSmsc());
    config.save(newFile);
  }

  public SmscSettings load(InputStream is) throws Exception {

    SmscSettings result = new SmscSettings();

    XmlConfig config = new XmlConfig();
    config.load(is);
    XmlConfigSection  smscConnectors = config.getSection("SMSCConnectors");
    Collection<Smsc> smscs = new LinkedList<Smsc>();
    for(XmlConfigSection s : smscConnectors.sections()) {
      Smsc smsc = new Smsc(s.getName());
      smsc.load(s);
      result.addSmsc(smsc);
    }
    result.setDefaultSmsc(smscConnectors.getString("default"));

    return result;
  }
//
//      <section name="SMSCConnectors">
//      <param name="default" type="string">SMSC0</param>
//
//      <section name="SMSC0">
//        <param name="host" type="string">niagara</param>
//        <param name="interfaceVersion" type="int">89</param>
//        <param name="password" type="string">infosme</param>
//        <param name="port" type="int">25004</param>
//        <param name="rangeOfAddress" type="string">1</param>
//        <param name="sid" type="string">infosme</param>
//        <param name="timeout" type="int">10</param>
//        <param name="ussdPushTag" type="int">131</param>
//        <param name="ussdPushVlrTag" type="int">153</param>
//      </section>
//    </section>
}
