package mobi.eyeline.dcpgw;

import mobi.eyeline.dcpgw.exeptions.InitializationException;
import mobi.eyeline.informer.util.config.XmlConfig;
import mobi.eyeline.informer.util.config.XmlConfigException;
import mobi.eyeline.informer.util.config.XmlConfigParam;
import mobi.eyeline.informer.util.config.XmlConfigSection;
import org.apache.log4j.Logger;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.Collection;
import java.util.Hashtable;
import java.util.Properties;

/**
 * Created by IntelliJ IDEA.
 * User: Stepanov Dmitry Nikolaevich
 * Date: 19.09.11
 * Time: 16:43
 */
public class ConfigurationManager {

    private static Logger log = Logger.getLogger(ConfigurationManager.class);

    private String smpp_server_config_file;
    private String smpp_endpoints_file;
    private String deliveries_file;

    public static final String CONNECTION_PREFIX = "smpp.sme.";

    public ConfigurationManager(String smpp_server_config_file, String smpp_endpoints_file, String deliveries_file){
        this.smpp_server_config_file = smpp_server_config_file;
        this.smpp_endpoints_file = smpp_endpoints_file;
        this.deliveries_file = deliveries_file;
    }

    public Properties loadSmppConfigurations() throws IOException, XmlConfigException {
        Properties properties = new Properties();
        properties.load(new FileInputStream(smpp_server_config_file));

        XmlConfig xmlConfig = new XmlConfig();

        xmlConfig.load(new File(smpp_endpoints_file));
        XmlConfigSection endpoints_section = xmlConfig.getSection("endpoints");
        Collection<XmlConfigSection> c = endpoints_section.sections();
        for(XmlConfigSection s : c) {
            String endpoint_name = s.getName();

            XmlConfigParam p = s.getParam("enabled");
            if (p == null || p.getBool()) {
                p = s.getParam("systemId");
                String systemId = p.getString();

                p = s.getParam("password");
                String password = p.getString();

                properties.setProperty(CONNECTION_PREFIX + systemId + ".password", password);
                log.debug("Load endpoint: name=" + endpoint_name + ", systemId=" + systemId + ", password=" + password + " .");
            } else {
                log.debug("Endpoint '" + endpoint_name + "' is disabled.");
            }
        }
        return properties;
    }

    public Hashtable<String, String> loadUsers() throws XmlConfigException {
        log.debug("Try to load users ...");
        Hashtable<String, String> t = new Hashtable<String, String>();

        XmlConfig xmlConfig = new XmlConfig();
        xmlConfig.load(new File(deliveries_file));

        XmlConfigSection users_section = xmlConfig.getSection("users");
        Collection<XmlConfigSection> c = users_section.sections();
        for (XmlConfigSection s : c) {
            String login = s.getName();
            XmlConfigParam p = s.getParam("password");
            String password = p.getString();
            t.put(login, password);
            log.debug("login: " + login + ", password: " + password);
        }
        log.debug("Users loaded.");
        return t;
    }

    public Hashtable<String, Provider> loadProviders() throws XmlConfigException {
        log.debug("Try to load providers ...");
        Hashtable<String, Provider> t = new Hashtable<String, Provider>();

        XmlConfig xmlConfig = new XmlConfig();
        xmlConfig.load(new File(deliveries_file));

        XmlConfigSection providers_section = xmlConfig.getSection("providers");
        Collection<XmlConfigSection> providers_collection = providers_section.sections();

        for (XmlConfigSection s : providers_collection) {

            Provider provider = new Provider();

            String name = s.getName();
            provider.setName(name);

            XmlConfigParam p = s.getParam("endpoint_ids");
            String[] endpoint_ids = p.getStringArray(",");
            provider.setEndpointIds(endpoint_ids);

            p = s.getParam("description");
            String description = p.getString();
            provider.setDescription(description);

            XmlConfigSection deliveries_section = s.getSection("deliveries");
            Collection<XmlConfigSection> deliveries_collection = deliveries_section.sections();

            for (XmlConfigSection d_s : deliveries_collection) {

                Delivery delivery = new Delivery();

                int delivery_id = Integer.parseInt(d_s.getName());
                delivery.setId(delivery_id);

                p = d_s.getParam("user");
                String user = p.getString();
                delivery.setUser(user);

                p = d_s.getParam("services_numbers");
                String[] services_numbers = p.getStringArray(",");
                delivery.setServicesNumbers(services_numbers);

                provider.addDelivery(delivery);
                log.debug("Load " + delivery.toString());
            }

            for (String endpoint_id : endpoint_ids) t.put(endpoint_id, provider);

            log.debug("Load " + provider.toString());
        }
        log.debug("Providers loaded.");
        return t;
    }

}
