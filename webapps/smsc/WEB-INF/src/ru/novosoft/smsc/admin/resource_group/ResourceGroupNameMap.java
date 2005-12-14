package ru.novosoft.smsc.admin.resource_group;

import org.apache.log4j.Category;

import java.io.FileInputStream;
import java.io.IOException;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.Properties;

/**
 * Created by Serge Lugovoy
 * Date: 12.12.2005
 * Time: 16:44:04
 */
public class ResourceGroupNameMap {
  static Category logger = Category.getInstance(ResourceGroupNameMap.class);
  protected static HashMap name2sme = new HashMap();
  protected static HashMap sme2name = new HashMap();

  public static void init(String mapFileName) {
    if( mapFileName == null || mapFileName.length() == 0 ) return;
    Properties props = new Properties();
    try {
      props.load(new FileInputStream(mapFileName));
    } catch (IOException e) {
      logger.warn("could not read rg map file: "+mapFileName);
    }
    for( Iterator it = props.entrySet().iterator(); it.hasNext(); ) {
      Map.Entry entry = (Map.Entry) it.next();
      name2sme.put(entry.getKey(), entry.getValue());
      sme2name.put(entry.getValue(), entry.getKey());
    }
  }

  public static String getNameBySme(String sme) {
    return (String) sme2name.get(sme);
  }

  public static String getSmeByName(String name) {
    return (String) name2sme.get(name);
  }

}
