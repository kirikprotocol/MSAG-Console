package ru.sibinco.lib.backend.sme;

import org.w3c.dom.*;
import org.xml.sax.SAXException;
import ru.sibinco.lib.backend.util.xml.Utils;

import javax.xml.parsers.ParserConfigurationException;
import java.io.IOException;
import java.io.PrintWriter;
import java.util.*;


/**
 * Created by IntelliJ IDEA. User: igork Date: 25.02.2004 Time: 20:23:43
 */
public class SmeManager
{
  private final Map smes;
//  private final String configFilename;

  public SmeManager(final String configFilename) throws IOException, ParserConfigurationException, SAXException
  {
//    this.configFilename = configFilename;
    smes = load(configFilename);
  }

  private Map load(final String configFilename) throws IOException, ParserConfigurationException, SAXException, NullPointerException
  {
    final Map result = new HashMap();
    final Document document = Utils.parse(configFilename);
    final NodeList records = document.getDocumentElement().getElementsByTagName("smerecord");
    for (int i = 0; i < records.getLength(); i++) {
      final Element smeRecord = (Element) records.item(i);
      final Sme sme = new Sme(smeRecord);
      result.put(sme.getId(), sme);
    }
    return Collections.synchronizedMap(result);
  }

  public PrintWriter store(final PrintWriter out)
  {
    final List values = new LinkedList(smes.values());
    Collections.sort(values, new Comparator()
    {
      public int compare(final Object o1, final Object o2)
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
