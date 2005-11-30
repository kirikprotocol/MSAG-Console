package ru.sibinco.inman.backend;

import org.apache.log4j.Category;
import org.xml.sax.SAXException;

import java.util.Map;
import java.util.HashMap;
import java.io.IOException;
import java.io.File;

import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.util.config.Config;

import javax.xml.parsers.ParserConfigurationException;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 29.11.2005
 * Time: 11:56:35
 * To change this template use File | Settings | File Templates.
 */
public class InManSmeContext
{
    private static final Map instances = new HashMap();
    private Category logger = Category.getInstance(this.getClass());

    public static synchronized InManSmeContext getInstance(final SMSCAppContext appContext, final String smeId)
            throws AdminException, ParserConfigurationException, IOException, SAXException, Config.WrongParamTypeException, Config.ParamNotFoundException
    {
        InManSmeContext instance = (InManSmeContext) instances.get(smeId);
        if (null == instance) {
            instance = new InManSmeContext(appContext, smeId);
            instances.put(smeId, instance);
        }
        return instance;
    }

    private final SMSCAppContext appContext;
    private Config config = null;

    private String smeId = "inman";
    InManSme inManSme = null;

    private InManSmeContext(final SMSCAppContext appContext, final String smeId)
            throws AdminException, ParserConfigurationException, SAXException, IOException, Config.WrongParamTypeException, Config.ParamNotFoundException
    {
        this.smeId = smeId;
        this.appContext = appContext;
        resetConfig();
        logger.info("New InManSmeContext instance created");
        inManSme = new InManSme(appContext.getHostsManager().getServiceInfo(this.smeId), config.getInt("port"));
    }

    public Config loadCurrentConfig()
            throws AdminException, IOException, SAXException, ParserConfigurationException
    {
        return new Config(new File(appContext.getHostsManager().getServiceInfo(smeId).getServiceFolder(),
                "conf" + File.separatorChar + "config.xml"));
    }

    public void resetConfig()
            throws AdminException, SAXException, ParserConfigurationException, IOException
    {
        config = loadCurrentConfig();
    }

    public Config getConfig() {
        return config;
    }

    public InManSme getInManSme() {
        return inManSme;
    }
}
