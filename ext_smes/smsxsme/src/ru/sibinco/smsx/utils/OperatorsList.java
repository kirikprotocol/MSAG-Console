package ru.sibinco.smsx.utils;

import com.eyeline.utils.config.ConfigException;
import com.eyeline.utils.config.xml.XmlConfigParam;
import com.eyeline.utils.config.xml.XmlConfigSection;
import com.eyeline.utils.tree.radix.TemplatesRTree;

/**
 * User: artem
 * Date: 09.07.2007
 */

public class OperatorsList {

  private final TemplatesRTree<String> operatorsTree;

  public OperatorsList(XmlConfigSection s) throws ConfigException {
    operatorsTree = new TemplatesRTree<String>();
    for (XmlConfigParam p : s.params())
      for (String mask : p.getStringArray(","))
        operatorsTree.put(mask, p.getName());
  }

  public String getOperatorByAddress(String address) {
    return operatorsTree.get(address);
  }
}
