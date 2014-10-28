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

public class LocaleManager {

  @SuppressWarnings("FieldCanBeLocal")
  private Logger logger = Logger.getLogger(LocaleManager.class);

  private final File configFile;
  private final ArrayList localesList;

  public LocaleManager(String config) throws Throwable, IOException, ParserConfigurationException, SAXException {
      localesList = new ArrayList();
      if (logger.isInfoEnabled()) logger.info("Use locales file '"+config+"'.");
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
    Locale locale = new Locale(language,country);
    boolean marker = false;
    Locale[] locales = Locale.getAvailableLocales();
    for (int i = 0; i<locales.length;i++)
      if (locales[i].equals(locale)) {
        marker = true;
        break;
      }
    if (!marker) throw new Exception("Locale with language \""+language+"\""+((country.length()>0)?" and country \""+country+"\"":"")+" doesn't supported by JVM");

    //check if messages_xx_xx.properties exists
    Locale ResultLocale = ResourceBundle.getBundle("locales.messages",locale).getLocale();
    if (!ResultLocale.equals(locale)) throw new Exception("There is no messages_"+language+(country.length()>0?"_"+country+".properties":".properties"));
       return new SCAGLocale(locale,icon);        
  }

  public ArrayList validate(String language_contry) throws Throwable {
     Locale defaultLocale = null;
     boolean countryPresent = (language_contry.indexOf("_")==-1)?false:true;
     if (countryPresent) {
        String[] lan_con = language_contry.split("_");
        defaultLocale = new Locale(lan_con[0],lan_con[1]);
     } else
       defaultLocale = new Locale(language_contry);
       for (Iterator i = localesList.iterator();i.hasNext();)
         if (((SCAGLocale)i.next()).locale.equals(defaultLocale)) return localesList;
       throw new Exception("Wrong default language parameter in web.xml - localization information for default language \""+defaultLocale.getLanguage()+"\""+((countryPresent)?(" and country \""+defaultLocale.getCountry()+"\""):"") +" not scpecified in locales.xml");
  }

  public static class SCAGLocale {
    public Locale locale;
    public String icon;
    public SCAGLocale(Locale locale, String icon) {
      this.locale = locale;
      this.icon = icon;
    }
  }
}
