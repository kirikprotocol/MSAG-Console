package ru.novosoft.smsc.admin.route;

import ru.novosoft.smsc.admin.config.ManagedConfigFile;

import java.io.InputStream;
import java.io.OutputStream;

/**
 * @author Artem Snopkov
 */
class RoutesConfig implements ManagedConfigFile<RouteSubjectSettings> {
  
  public void save(InputStream oldFile, OutputStream newFile, RouteSubjectSettings conf) throws Exception {
    //To change body of implemented methods use File | Settings | File Templates.
  }

  public RouteSubjectSettings load(InputStream is) throws Exception {
    return null;  //To change body of implemented methods use File | Settings | File Templates.
  }
}
