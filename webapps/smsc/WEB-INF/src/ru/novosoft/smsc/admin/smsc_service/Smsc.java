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
import ru.novosoft.smsc.admin.profiler.Profile;
import ru.novosoft.smsc.admin.alias.AliasSet;
import ru.novosoft.smsc.admin.route.RouteList;
import ru.novosoft.smsc.admin.route.SMEList;
import ru.novosoft.smsc.admin.route.SubjectList;
import ru.novosoft.smsc.admin.route.Mask;
import ru.novosoft.smsc.admin.service.*;
import ru.novosoft.smsc.util.config.Config;
import ru.novosoft.smsc.util.config.ConfigManager;
import ru.novosoft.smsc.util.xml.Utils;
import ru.novosoft.smsc.jsp.util.tables.impl.ProfileDataSource;
import ru.novosoft.smsc.jsp.util.tables.impl.ProfileQuery;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;

import javax.xml.parsers.FactoryConfigurationError;
import javax.xml.parsers.ParserConfigurationException;
import java.io.*;
import java.util.HashMap;


public class Smsc extends Service
{
  private Component smsc_component = null;
  private Method apply_routes_method = null;
  private Method apply_aliases_method = null;
  private Method lookup_profile_method = null;
  private Method update_profile_method = null;
  private ConfigManager configManager = null;

  private SMEList smes = null;
  private RouteList routes = null;
  private SubjectList subjects = null;
  private AliasSet aliases = null;
  private ProfileDataSource profileDataSource = null;

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
    lookup_profile_method = (Method) smsc_component.getMethods().get("lookup_profile");
    update_profile_method = (Method) smsc_component.getMethods().get("update_profile");

    try {
      final File smscConfFolder = getSmscConfFolder();
      Document smesDoc = Utils.parse(new FileInputStream(new File(smscConfFolder, "sme.xml")));
      Document routesDoc = Utils.parse(new FileInputStream(new File(smscConfFolder, "routes.xml")));
      Document aliasesDoc = Utils.parse(new FileInputStream(new File(smscConfFolder, "aliases.xml")));
      smes = new SMEList(smesDoc.getDocumentElement());
      subjects = new SubjectList(routesDoc.getDocumentElement(), smes);
      routes = new RouteList(routesDoc.getDocumentElement(), subjects, smes);
      aliases = new AliasSet(aliasesDoc.getDocumentElement());
      profileDataSource = new ProfileDataSource(
          configManager.getConfig().getString("profiler.jdbc.source"),
          configManager.getConfig().getString("profiler.jdbc.driver"),
          configManager.getConfig().getString("profiler.jdbc.user"),
          configManager.getConfig().getString("profiler.jdbc.password")
      );
    } catch (FactoryConfigurationError error) {
      logger.error("Couldn't configure xml parser factory", error);
      throw new AdminException("Couldn't configure xml parser factory: " + error.getMessage());
    } catch (ParserConfigurationException e) {
      logger.error("Couldn't configure xml parser", e);
      throw new AdminException("Couldn't configure xml parser: " + e.getMessage());
    } catch (SAXException e) {
      logger.error("Couldn't parse", e);
      throw new AdminException("Couldn't parse: " + e.getMessage());
    } catch (IOException e) {
      logger.error("Couldn't read", e);
      throw new AdminException("Couldn't read: " + e.getMessage());
    } catch (NullPointerException e) {
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

    try {
      final File smscConfFolder = getSmscConfFolder();

      storeSmes(new PrintWriter(new FileOutputStream(new File(smscConfFolder, "sme.xml")), true)).close();
      storeRoutes(new PrintWriter(new FileOutputStream(new File(smscConfFolder, "routes.xml")), true)).close();

      call(smsc_component, apply_routes_method, Type.Types[Type.StringType], new HashMap());
    } catch (Config.ParamNotFoundException e) {
      throw new AdminException("Couldn't apply_routes new settings: Administration application misconfigured: " + e.getMessage());
    } catch (Config.WrongParamTypeException e) {
      throw new AdminException("Couldn't apply_routes new settings: Administration application misconfigured: " + e.getMessage());
    } catch (FileNotFoundException e) {
      throw new AdminException("Couldn't apply_routes new settings: Couldn't write to destination config file: " + e.getMessage());
    }
  }

  public void applyAliases()
      throws AdminException
  {

    try {
      final File smscConfFolder = getSmscConfFolder();

      storeAliases(new PrintWriter(new FileOutputStream(new File(smscConfFolder, "aliases.xml")), true)).close();

      call(smsc_component, apply_aliases_method, Type.Types[Type.StringType], new HashMap());
    } catch (Config.ParamNotFoundException e) {
      throw new AdminException("Couldn't apply_routes new settings: Administration application misconfigured: " + e.getMessage());
    } catch (Config.WrongParamTypeException e) {
      throw new AdminException("Couldn't apply_routes new settings: Administration application misconfigured: " + e.getMessage());
    } catch (FileNotFoundException e) {
      throw new AdminException("Couldn't apply_routes new settings: Couldn't write to destination config file: " + e.getMessage());
    }
  }

  public AliasSet getAliases()
  {
    return aliases;
  }

  public Profile lookupProfile(Mask mask)
      throws AdminException
  {
    HashMap args = new HashMap();
    args.put("address", mask.getMask());
    Object result = call(smsc_component, lookup_profile_method, Type.Types[Type.StringType], args);
    if (result instanceof String)
      return new Profile(mask, (String) result);
    else
      throw new AdminException("Error in response");
  }

  public void updateProfile(Mask mask, Profile newProfile)
      throws AdminException
  {
    HashMap args = new HashMap();
    args.put("address", mask.getMask());
    args.put("profile", newProfile.getStringRepresentation());
    call(smsc_component, update_profile_method, Type.Types[Type.BooleanType], args);
  }

  public QueryResultSet queryProfiles(ProfileQuery query)
      throws AdminException
  {
    return profileDataSource.query(query);
  }
}
