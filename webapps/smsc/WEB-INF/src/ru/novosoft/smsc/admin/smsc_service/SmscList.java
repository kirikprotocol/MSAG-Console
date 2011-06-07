package ru.novosoft.smsc.admin.smsc_service;

import org.apache.log4j.Category;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.Constants;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.util.StringEncoderDecoder;
import ru.novosoft.smsc.util.WebAppFolders;
import ru.novosoft.smsc.util.config.Config;

import java.io.File;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.Set;

/**
 * Created by IntelliJ IDEA.
 * User: starkom
 * Date: 07.07.2005
 * Time: 14:05:08
 * To change this template use File | Settings | File Templates.
 */

public final class SmscList {
    private final Category logger = Category.getInstance(this.getClass());
    private Smsc smsc = null;
    public static final Map nodeName2Id = new HashMap();
    public static final Map id2NodeName = new HashMap();

    public static byte getNodeId(String nodename) {
        String idStr = (String) nodeName2Id.get(nodename);
        if (idStr != null) return Byte.parseByte(idStr);
        else return 0;
    }

    public static String getNodeFromId(byte nodeId) {
        return (String) id2NodeName.get(String.valueOf(nodeId));
    }

    public SmscList(final Config webappConfig, SMSCAppContext smscAppContext) throws AdminException {
        Set nodeNames = webappConfig.getSectionChildSectionNames("nodes");
        logger.debug("Initializing SMSC list ");

        try {
            for (Iterator i = nodeNames.iterator(); i.hasNext();) {
                String encodedName = (String) i.next();
                String nodeName = StringEncoderDecoder.decodeDot(encodedName.substring(encodedName.lastIndexOf('.') + 1));
                String nodeId = webappConfig.getString(encodedName + ".id");
                nodeName2Id.put(nodeName, nodeId);
                id2NodeName.put(nodeId, nodeName);
            }
            String smscConfFolder = webappConfig.getString("smsc.config folder");
            WebAppFolders.setSmscConfFolder(new File(smscConfFolder));

            int timeout = 180000;
            if (webappConfig.containsParameter("smsc.timeout")) {
              timeout = webappConfig.getInt("smsc.timeout");
            }

            if (webappConfig.containsParameter("smsx.host") && webappConfig.containsParameter("smsx.port")) {
              String distrHost = webappConfig.getString("smsx.host");
              int distrPort = webappConfig.getInt("smsx.port");
              if (logger.isDebugEnabled())
                logger.debug("Use separate host and port for distr lists manager: " + distrHost + ":" + distrPort);

              smsc = new Smsc(Constants.SMSC_SME_ID, webappConfig.getString("smsc.host"), webappConfig.getInt("smsc.port"), timeout,
                  smscConfFolder, smscAppContext, distrHost, distrPort);
            } else {
              smsc = new Smsc(Constants.SMSC_SME_ID, webappConfig.getString("smsc.host"), webappConfig.getInt("smsc.port"), timeout,
                  smscConfFolder, smscAppContext);
            }
            logger.debug("SMSC added");
        }
        catch (AdminException e) {
            logger.error("Couldn't add SMSC \"", e);
            throw e;
        }
        catch (Config.ParamNotFoundException e) {
            logger.debug("Misconfigured SMSC \"", e);
            throw new AdminException("Miscofigured SMSC");
        }
        catch (Config.WrongParamTypeException e) {
            logger.debug("Misconfigured SMSC \"", e);
            throw new AdminException("Miscofigured SMSC");
        }
        logger.debug("SMSC list initialized");
        smscAppContext.setSmsc(smsc);
    }

    public final Smsc getSmsc() {
        return smsc;
    }

    public final void applyConfig() throws AdminException {
        try {
            getSmsc().getSmscConfig().save();
	    getSmsc().applyConfig();
        }
        catch (Exception e) {
            e.printStackTrace();
            throw new AdminException("Couldn't save Smsc config");
        }
    }

}