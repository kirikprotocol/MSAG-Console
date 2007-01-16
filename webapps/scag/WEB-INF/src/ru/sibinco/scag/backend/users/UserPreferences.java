package ru.sibinco.scag.backend.users;

import org.w3c.dom.NodeList;
import org.w3c.dom.Element;

import java.util.*;

/**
 * Created by IntelliJ IDEA.
 * User: dym
 * Date: 20.12.2006
 * Time: 12:36:06
 * To change this template use File | Settings | File Templates.
 */
public class UserPreferences {
  TreeMap monPrefs = new TreeMap();

  public UserPreferences() {
    //default values
    //performance monitor
    monPrefs.put("perfmon.http.pixPerSecond","4");
    monPrefs.put("perfmon.http.scale","80");
    monPrefs.put("perfmon.http.block","8");
    monPrefs.put("perfmon.smpp.pixPerSecond","4");
    monPrefs.put("perfmon.smpp.scale","80");
    monPrefs.put("perfmon.smpp.block","8");
    monPrefs.put("perfmon.vLightGrid","4");
    monPrefs.put("perfmon.vMinuteGrid","6");
    //center monitor
    monPrefs.put("scmon.graph.scale","2");
    monPrefs.put("scmon.graph.grid","2");
    monPrefs.put("scmon.graph.higrid","10");
    monPrefs.put("scmon.graph.head","20");
    monPrefs.put("scmon.max.speed","50");
    //service points monitor
    monPrefs.put("svcmon.graph.scale","2");
    monPrefs.put("svcmon.graph.grid","2");
    monPrefs.put("svcmon.graph.higrid","10");
    monPrefs.put("svcmon.graph.head","20");
    monPrefs.put("svcmon.max.speed","50");    
  }

  public UserPreferences(NodeList values) {
    this();
    if (values != null) {
			for (int i = 0; i < values.getLength(); i++) {
				Element elem = (Element) values.item(i);
				String name = elem.getAttribute("name");
				String value = elem.getAttribute("value");
        monPrefs.put(name, value);
			}
		}
  }

  public UserPreferences(Map monPrefs) {
    this.monPrefs.putAll(monPrefs);
  }

  public String getXmlText() {
    StringBuffer result = new StringBuffer();
    for(Iterator i = monPrefs.entrySet().iterator(); i.hasNext();) {
      Map.Entry entry = (Map.Entry) i.next();
      result.append("\t\t<pref name=\"" + entry.getKey() + "\" value=\"" + entry.getValue() + "\"/>\n");
    }
    return result.toString();
  }

  public Map getMonPrefs() {
    return monPrefs;
  }

}
