package ru.sibinco.lib.backend.sme;

import org.apache.log4j.Logger;
import org.w3c.dom.*;
import org.xml.sax.SAXException;
import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.backend.util.xml.Utils;

import javax.xml.parsers.ParserConfigurationException;
import java.io.*;
import java.util.*;


/**
 * Created by IntelliJ IDEA. User: igork Date: 25.02.2004 Time: 20:23:43
 */
public class SmeManager
{
  private Logger logger = Logger.getLogger(this.getClass());

  private final Map smes = Collections.synchronizedMap(new HashMap());
  private final String configFilename;

  public SmeManager(final String configFilename) throws IOException, ParserConfigurationException, SAXException
  {
    this.configFilename = configFilename;
  }

  public synchronized void init() throws IOException, ParserConfigurationException, SAXException
  {
    smes.clear();
    final Document document = Utils.parse(configFilename);
    final NodeList records = document.getDocumentElement().getElementsByTagName("smerecord");
    for (int i = 0; i < records.getLength(); i++) {
      final Element smeRecord = (Element) records.item(i);
      final Sme sme = createSme(smeRecord);
      smes.put(sme.getId(), sme);
    }
  }

  protected Sme createSme(final Element smeRecord)
  {
    return new Sme(smeRecord);
  }

  public synchronized PrintWriter store(final PrintWriter out)
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

  public void apply() throws SibincoException
  {
    try {
      store(new PrintWriter(new FileWriter(configFilename))).close();
    } catch (IOException e) {
      logger.error("Couldn't save SMEs config", e);
      throw new SibincoException("Couldn't save SMEs config", e);
    }
  }
}
