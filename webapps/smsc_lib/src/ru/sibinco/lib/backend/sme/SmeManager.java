package ru.sibinco.lib.backend.sme;

import org.w3c.dom.*;
import org.xml.sax.SAXException;
import ru.sibinco.lib.backend.util.xml.Utils;

import javax.xml.parsers.ParserConfigurationException;
import java.io.IOException;
import java.io.PrintWriter;
import java.util.*;


/**
 * Created by IntelliJ IDEA.
 * User: igork
 * Date: 25.02.2004
 * Time: 20:23:43
 */
public class SmeManager
{
  private final Map smes;
//  private final String configFilename;

  public SmeManager(String configFilename) throws IOException, ParserConfigurationException, SAXException
  {
//    this.configFilename = configFilename;
    smes = load(configFilename);
  }

  private Map load(String configFilename) throws IOException, ParserConfigurationException, SAXException, NullPointerException
  {
    Map result = new HashMap();
    Document document = Utils.parse(configFilename);
    NodeList records = document.getDocumentElement().getElementsByTagName("smerecord");
    for (int i = 0; i < records.getLength(); i++) {
      Element smeRecord = (Element) records.item(i);
      Sme sme = new Sme(smeRecord);
      result.put(sme.getId(), sme);
    }
    return Collections.synchronizedMap(result);
  }

  public PrintWriter store(PrintWriter out)
  {
    List values = new LinkedList(smes.values());
    Collections.sort(values, new Comparator()
    {
      public int compare(Object o1, Object o2)
      {
        final Sme s1 = (Sme) o1;
        final Sme s2 = (Sme) o2;
        return s1.getId().compareTo(s2.getId());
      }
    });
    for (Iterator i = values.iterator(); i.hasNext();)
      ((Sme) i.next()).store(out);
    return out;
  }

  public Map getSmes()
  {
    return smes;
  }
}
