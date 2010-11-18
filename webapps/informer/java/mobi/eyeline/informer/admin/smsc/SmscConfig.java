package mobi.eyeline.informer.admin.smsc;

import mobi.eyeline.informer.admin.util.config.ManagedConfigFile;
import mobi.eyeline.informer.util.config.XmlConfig;
import mobi.eyeline.informer.util.config.XmlConfigSection;

import java.io.InputStream;
import java.io.OutputStream;
import java.util.HashSet;
import java.util.Set;

/**
 * @author Aleksandr Khalitov
 */
class SmscConfig implements ManagedConfigFile<SmscSettings> {

  public void save(InputStream oldFile, OutputStream newFile, SmscSettings conf) throws Exception {
    XmlConfig config = new XmlConfig();
    config.load(oldFile);

    XmlConfigSection smscConnectors = config.getOrCreateSection("SMSCConnectors");

    Set<String> newSections = new HashSet<String>();

    for (Smsc s : conf.getSmscs()) {
      XmlConfigSection section = smscConnectors.getOrCreateSection(s.getName());
      s.save(section);
      newSections.add(section.getName());
    }

    for (XmlConfigSection section : smscConnectors.sections()) {
      if (!newSections.contains(section.getName())) {
        smscConnectors.removeSection(section);
      }
    }

    config.addSection(smscConnectors);
    smscConnectors.setString("default", conf.getDefaultSmsc());
    config.save(newFile);
  }

  public SmscSettings load(InputStream is) throws Exception {

    SmscSettings result = new SmscSettings();

    XmlConfig config = new XmlConfig();
    config.load(is);
    XmlConfigSection smscConnectors = config.getSection("SMSCConnectors");
    for (XmlConfigSection s : smscConnectors.sections()) {
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
