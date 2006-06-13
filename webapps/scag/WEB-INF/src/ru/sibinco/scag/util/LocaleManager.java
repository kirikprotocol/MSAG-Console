package ru.sibinco.scag.util;

import ru.sibinco.lib.backend.util.xml.Utils;

import javax.xml.parsers.ParserConfigurationException;
import java.io.IOException;
import java.io.File;
import java.io.FileReader;
import java.util.*;

import org.xml.sax.SAXException;
import org.w3c.dom.Document;
import org.w3c.dom.NodeList;
import org.w3c.dom.Element;
import org.apache.log4j.Logger;


/**
 * Created by IntelliJ IDEA.
 * User: dym
 * Date: 08.06.2006
 * Time: 12:54:01
 * To change this template use File | Settings | File Templates.
 */
public class LocaleManager {
  private Logger logger = Logger.getLogger(LocaleManager.class);
  private final File configFile;
  private final ArrayList localesList;

  public LocaleManager(String config) throws Throwable, IOException, ParserConfigurationException, SAXException {
      localesList = new ArrayList();
      configFile = new File(config);
      try {
          load();
      } catch (IOException e) {
          e.printStackTrace();
          logger.warn(e.getMessage());
          throw new IOException(e.getMessage());
      } catch (ParserConfigurationException e) {
          e.printStackTrace();
          logger.warn(e.getMessage());
          throw new ParserConfigurationException(e.getMessage());
      } catch (SAXException e) {
          e.printStackTrace();
          logger.warn(e.getMessage());
          throw new SAXException(e.getMessage());
      }
  }

  private void load() throws Throwable, IOException, ParserConfigurationException, SAXException {
    Document document = Utils.parse(new FileReader(configFile));
    NodeList locales = document.getElementsByTagName("locale");
    for (int i=0;i<locales.getLength();i++){
      SCAGLocale locale = validate((Element)locales.item(i));
      localesList.add(locale);
    }

  }

  public ArrayList getLocaleList() {
     return localesList;
  }

  private SCAGLocale validate(Element localeElement) throws Throwable {
    String language = localeElement.getAttribute("language");
    if (language.equals("")) throw new Exception("attribute \"language\" must be specified");
    String icon = localeElement.getAttribute("icon");
    if (icon.equals("")) throw new Exception("attribute \"icon\" must be specified");
    String country = localeElement.getAttribute("country");
    //check in ISO 639
    String[] languages = Locale.getISOLanguages();
    String[] countries;
    if (!country.equals("")) {
     //check in ISO 3166
     countries= Locale.getISOCountries();
     //if (...) throws Exception
    }
    //check if messages_xx_xx.properties exists
    boolean isDefaultLocale = false;
    Locale locale, defaultLocale;
    locale = new Locale(language,country);
    if (country.equals("")) {
      defaultLocale= new Locale(Locale.getDefault().getLanguage());
    } else
      defaultLocale=Locale.getDefault();
    if (locale.equals(defaultLocale)) isDefaultLocale = true;
    ResourceBundle rb = ResourceBundle.getBundle("locales.messages",locale);
    //System.out.println(language+"####"+rb);
    //System.out.println(language+"????"+ResourceBundle.getBundle("locales.messages", Locale.getDefault()));
    if (rb == ResourceBundle.getBundle("locales.messages", Locale.getDefault())) {
      if (!isDefaultLocale) throw new Exception("There is no messages_"+language+(country.length()>0?"_"+country+".properties":".properties"));
    }
    return new SCAGLocale(locale,icon);
  }

  public ArrayList validate(String language) throws Throwable {
     for (Iterator i = localesList.iterator();i.hasNext();)
       if (((SCAGLocale)i.next()).locale.getLanguage().equals(language)) return localesList;
     throw new Exception("localization information for default language \""+language+"\" not scpecified in locales.xml");
  }

  public class SCAGLocale {
    public Locale locale;
    public String icon;
    public SCAGLocale(Locale locale, String icon) {
      this.locale = locale;
      this.icon = icon;
    }
  }
}
