/*
 * Author: igork
 * Date: 27.05.2002
 * Time: 18:59:55
 */
package ru.novosoft.smsc.admin.smsc_service;

import org.apache.log4j.Category;
import org.w3c.dom.Document;
import org.xml.sax.SAXException;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.alias.AliasSet;
import ru.novosoft.smsc.admin.route.RouteList;
import ru.novosoft.smsc.admin.route.SMEList;
import ru.novosoft.smsc.admin.route.SubjectList;
import ru.novosoft.smsc.admin.service.*;
import ru.novosoft.smsc.util.config.Config;
import ru.novosoft.smsc.util.config.ConfigManager;
import ru.novosoft.smsc.util.xml.Utils;

import javax.xml.parsers.FactoryConfigurationError;
import javax.xml.parsers.ParserConfigurationException;
import java.io.*;
import java.util.HashMap;


public class Smsc extends Service
{
  private Component smsc_component = null;
  private Method apply_routes_method = null;
  private Method apply_aliases_method = null;
  private ConfigManager configManager = null;

  private SMEList smes = null;
  private RouteList routes = null;
  private SubjectList subjects = null;
  private AliasSet aliases = null;

  private Category logger = Category.getInstance(this.getClass());

  public Smsc(ConfigManager configManager)
          throws AdminException, Config.ParamNotFoundException, Config.WrongParamTypeException
  {
    super(new ServiceInfo("SMSC",
                          configManager.getConfig().getString("smsc.host"),
                          configManager.getConfig().getInt("smsc.port"),
                          "", null));

    this.configManager = configManager;

    refreshComponents();
    smsc_component = (Component) getInfo().getComponents().get("SMSC");
    apply_routes_method = (Method) smsc_component.getMethods().get("apply_routes");
    apply_aliases_method = (Method) smsc_component.getMethods().get("apply_aliases");

    try
    {
      final File smscConfFolder = getSmscConfFolder();
      Document smesDoc = Utils.parse(new FileInputStream(new File(smscConfFolder, "sme.xml")));
      Document routesDoc = Utils.parse(new FileInputStream(new File(smscConfFolder, "routes.xml")));
      Document aliasesDoc = Utils.parse(new FileInputStream(new File(smscConfFolder, "aliases.xml")));
      smes = new SMEList(smesDoc.getDocumentElement());
      subjects = new SubjectList(routesDoc.getDocumentElement(), smes);
      routes = new RouteList(routesDoc.getDocumentElement(), subjects, smes);
      aliases = new AliasSet(aliasesDoc.getDocumentElement());
    }
    catch (FactoryConfigurationError error)
    {
      logger.error("Couldn't configure xml parser factory", error);
      throw new AdminException("Couldn't configure xml parser factory: " + error.getMessage());
    }
    catch (ParserConfigurationException e)
    {
      logger.error("Couldn't configure xml parser", e);
      throw new AdminException("Couldn't configure xml parser: " + e.getMessage());
    }
    catch (SAXException e)
    {
      logger.error("Couldn't parse", e);
      throw new AdminException("Couldn't parse: " + e.getMessage());
    }
    catch (IOException e)
    {
      logger.error("Couldn't read", e);
      throw new AdminException("Couldn't read: " + e.getMessage());
    }
    catch (NullPointerException e)
    {
      logger.error("Couldn't parse", e);
      throw new AdminException("Couldn't parse: " + e.getMessage());
    }
  }

  public RouteList getRoutes()
  {
    return routes;
  }

  public SMEList getSmes()
  {
    return smes;
  }

  public SubjectList getSubjects()
  {
    return subjects;
  }

  private File getSmscConfFolder()
    throws Config.ParamNotFoundException, Config.WrongParamTypeException
  {
    return new File(configManager.getConfig().getString("system.webapp folder"), "WEB-INF/smsc/conf");
  }

  protected PrintWriter storeSmes(PrintWriter out)
  {
    out.println("<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>");
    out.println("<!DOCTYPE records SYSTEM \"file://SmeRecords.dtd\">");
    out.println();
    out.println("<records>");

    smes.store(out);

    out.println("</records>");
    return out;
  }

  protected PrintWriter storeRoutes(PrintWriter out)
  {
    out.println("<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>");
    out.println("<!DOCTYPE routes SYSTEM \"file://routes.dtd\">");
    out.println();
    out.println("<routes>");
    subjects.store(out);
    routes.store(out);
    out.println("</routes>");
    return out;
  }

  protected PrintWriter storeAliases(PrintWriter out)
  {
    out.println("<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>");
    out.println("<!DOCTYPE aliases SYSTEM \"file://AliasRecords.dtd\">");
    out.println();
    out.println("<aliases>");
    aliases.store(out);
    out.println("</aliases>");
    return out;
  }

  public void applyRoutes()
          throws AdminException
  {

    try
    {
      final File smscConfFolder = getSmscConfFolder();

      storeSmes(new PrintWriter(new FileOutputStream(new File(smscConfFolder, "sme.xml")), true)).close();
      storeRoutes(new PrintWriter(new FileOutputStream(new File(smscConfFolder, "routes.xml")), true)).close();

      call(smsc_component, apply_routes_method, Type.Types[Type.StringType], new HashMap());
    }
    catch (Config.ParamNotFoundException e)
    {
      throw new AdminException("Couldn't apply_routes new settings: Administration application misconfigured: " + e.getMessage());
    }
    catch (Config.WrongParamTypeException e)
    {
      throw new AdminException("Couldn't apply_routes new settings: Administration application misconfigured: " + e.getMessage());
    }
    catch (FileNotFoundException e)
    {
      throw new AdminException("Couldn't apply_routes new settings: Couldn't write to destination config file: " + e.getMessage());
    }
  }

  public void applyAliases()
          throws AdminException
  {

    try
    {
      final File smscConfFolder = getSmscConfFolder();

      storeAliases(new PrintWriter(new FileOutputStream(new File(smscConfFolder, "aliases.xml")), true)).close();

      call(smsc_component, apply_aliases_method, Type.Types[Type.StringType], new HashMap());
    }
    catch (Config.ParamNotFoundException e)
    {
      throw new AdminException("Couldn't apply_routes new settings: Administration application misconfigured: " + e.getMessage());
    }
    catch (Config.WrongParamTypeException e)
    {
      throw new AdminException("Couldn't apply_routes new settings: Administration application misconfigured: " + e.getMessage());
    }
    catch (FileNotFoundException e)
    {
      throw new AdminException("Couldn't apply_routes new settings: Couldn't write to destination config file: " + e.getMessage());
    }
  }

  public AliasSet getAliases()
  {
    return aliases;
  }
}
