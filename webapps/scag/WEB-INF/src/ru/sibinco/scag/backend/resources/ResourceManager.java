package ru.sibinco.scag.backend.resources;

import org.apache.log4j.Logger;
import org.xml.sax.SAXException;

import javax.xml.parsers.ParserConfigurationException;
import java.io.*;
import java.util.HashMap;
import java.util.Map;


/**
 * Created by igork
 * Date: 19.05.2004
 * Time: 18:26:58
 */
public class ResourceManager
{
  private Logger logger = Logger.getLogger(this.getClass());
  private Map resources = new HashMap();
  private static final String RESOURCE_PREFIX = "resources_";
  private static final String RESOURCE_SUFFIX = ".xml";
  private static final int RESOURCE_FILENAME_LENGTH = RESOURCE_PREFIX.length() + 5 + RESOURCE_SUFFIX.length();

  public ResourceManager(File resourcesDirectory) throws IOException, ParserConfigurationException, SAXException
  {
    if (resourcesDirectory.exists() && resourcesDirectory.isDirectory() && resourcesDirectory.canRead()) {
      final File[] resourceFiles = resourcesDirectory.listFiles(new FilenameFilter()
      {
        public boolean accept(File dir, String name)
        {
          if (name.length() == RESOURCE_FILENAME_LENGTH && name.startsWith(RESOURCE_PREFIX) && name.endsWith(RESOURCE_SUFFIX)) {
            logger.debug("Accepted resource file: \"" + name + "\" in \"" + dir.getAbsolutePath() + "\"");
            //System.out.println("Accepted resource file: \"" + name + "\" in \"" + dir.getAbsolutePath() + "\"");
            return true;
          } else
            return false;
        }
      });
      if (resourceFiles != null && resourceFiles.length > 0) {
        //System.out.println("RESOURCE_PREFIX.length() = " + RESOURCE_PREFIX.length());
        for (int i = 0; i < resourceFiles.length; i++) {
          File resourceFile = resourceFiles[i];
          final int res_pref_len = RESOURCE_PREFIX.length();
          Resource resource = null;
          try {
            resource = new Resource(resourceFile, resourceFile.getName().substring(res_pref_len, res_pref_len + 5));
          } catch (IOException e) {
            e.printStackTrace();//logger.warn(e.getMessage());  //To change body of catch statement use File | Settings | File Templates.
           throw new IOException(e.getMessage());
          } catch (ParserConfigurationException e) {
            e.printStackTrace();//logger.warn(e.getMessage());  //To change body of catch statement use File | Settings | File Templates.
           throw new ParserConfigurationException(e.getMessage());
          } catch (SAXException e) {
            e.printStackTrace();//logger.warn(e.getMessage());  //To change body of catch statement use File | Settings | File Templates.
           throw new SAXException(e.getMessage());
          }
          resources.put(resource.getLocaleName(), resource);
        }
      }
    }
  }

  public Map getResources()
  {
    return resources;
  }
}
