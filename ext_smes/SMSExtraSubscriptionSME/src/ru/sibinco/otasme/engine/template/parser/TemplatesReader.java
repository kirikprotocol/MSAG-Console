package ru.sibinco.otasme.engine.template.parser;

import org.xml.sax.SAXException;
import ru.sibinco.otasme.engine.template.MacroRegion;
import ru.sibinco.otasme.engine.template.Templates;

import javax.xml.parsers.ParserConfigurationException;
import javax.xml.parsers.SAXParser;
import javax.xml.parsers.SAXParserFactory;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.File;

/**
 * User: artem
 * Date: 25.10.2006
 */

public class TemplatesReader {

  public static Templates readTemplates(String fileName) throws TemplatesReaderException {

    try {
      final File f = new File(fileName);
      if (f.canRead()) {
        final Templates templates = new Templates();

        final SAXParser parser = SAXParserFactory.newInstance().newSAXParser();
        parser.parse(new FileInputStream(f), new TemplatesParser(templates));

        return templates;
      }
      return null;
    } catch (IOException e) {
      throw new TemplatesReaderException(e);
    } catch (ParserConfigurationException e) {
      throw new TemplatesReaderException(e);
    } catch (SAXException e) {
      throw new TemplatesReaderException(e);
    } catch (Exception e) {
      throw new TemplatesReaderException(e);
    }
  }

  public static class TemplatesReaderException extends Exception {
    public TemplatesReaderException(Throwable cause) {
      super(cause);
    }
  }

  public static void main(String args[]) {
    try {
      final Templates t = readTemplates("config\\templates.xml");

      final MacroRegion region = t.getMacroRegionByAbonentNumber("9607891901");
      if (region == null)
        System.out.println("Region not found");
      else
        System.out.println(region.getName());

    } catch (TemplatesReaderException e) {
      e.printStackTrace();
    }

  }
}
