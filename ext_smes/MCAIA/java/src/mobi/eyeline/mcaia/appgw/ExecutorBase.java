package mobi.eyeline.mcaia.appgw;

import mobi.eyeline.smpp.appgw.scenario.Executor;
import mobi.eyeline.smpp.appgw.scenario.ScenarioInitializationException;
import mobi.eyeline.smpp.appgw.scenario.resources.ScenarioResourceBundle;

import java.util.Properties;

/**
 * Created by Serge Lugovoy
 * Date: May 27, 2010
 * Time: 2:44:23 PM
 */
public abstract class ExecutorBase implements Executor {
  ScenarioResourceBundle resources;
  Properties initParams;

  public void init(Properties properties) throws ScenarioInitializationException {
    initParams = properties;
    resources = (ScenarioResourceBundle) properties.get("resources");
    if( resources == null ) throw new ScenarioInitializationException("Resource bundle required as \"resources\" param");
  }

  public String getParam( String name ) throws ScenarioInitializationException {
    String s = initParams.getProperty(name);
    if( s == null ) {
      throw new ScenarioInitializationException("Init parameter "+name+" not specified for executor "+getClass().getName());
    }
    return s.trim();
  }

}
