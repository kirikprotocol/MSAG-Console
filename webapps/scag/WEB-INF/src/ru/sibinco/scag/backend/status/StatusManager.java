/*
 * Copyright (c) 2006 SibInco Inc. All Rights Reserved.
 */

package ru.sibinco.scag.backend.status;

import org.apache.log4j.Logger;
import org.xml.sax.SAXException;

import javax.xml.parsers.ParserConfigurationException;
import java.util.Map;
import java.util.Collections;
import java.util.HashMap;
import java.util.Date;
import java.io.IOException;
import java.io.File;

/**
 * The <code>StatusManager</code> class represents
 * <p><p/>
 * Date: 27.01.2006
 * Time: 15:02:06
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class StatusManager {

    private Logger logger = Logger.getLogger(this.getClass());

    private final File statusFolder;

    private final Map statMessages = Collections.synchronizedMap(new HashMap());

    public StatusManager(final File statusFolder) {
        if(!statusFolder.exists()){
            statusFolder.mkdirs();
        }
        this.statusFolder = statusFolder;
    }

    public synchronized void init() throws IOException, ParserConfigurationException, SAXException {
        statMessages.clear();
    }

    public synchronized Map getStatMessages() {
        return statMessages;
    }

}
