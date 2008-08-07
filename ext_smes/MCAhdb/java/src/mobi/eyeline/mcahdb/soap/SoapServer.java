package mobi.eyeline.mcahdb.soap;

import mobi.eyeline.mcahdb.engine.EventsFetcher;
import mobi.eyeline.mcahdb.soap.missedcallservice.MissedCallServiceMessageReceiverInOut;
import mobi.eyeline.mcahdb.soap.missedcallservice.MissedCallServiceSkeleton;
import org.apache.axis2.AxisFault;
import org.apache.axis2.context.ConfigurationContextFactory;
import org.apache.axis2.description.AxisService;
import org.apache.axis2.description.WSDL2Constants;
import org.apache.axis2.engine.AxisServer;
import org.apache.axis2.engine.MessageReceiver;

import java.io.IOException;
import java.util.HashMap;
import java.util.Map;

import com.eyeline.utils.config.xml.XmlConfig;
import com.eyeline.utils.config.ConfigException;

/**
 * User: artem
 * Date: 04.08.2008
 *
 *
 */

public class SoapServer extends AxisServer {

  public SoapServer(SoapConfig config, EventsFetcher fetcher) throws IOException, ConfigException {
    super(false); // run on deploy

    ServiceContext.init(fetcher, config);

    this.configContext = ConfigurationContextFactory.createConfigurationContextFromFileSystem(null, "conf/axis2.xml");

    Map<String, MessageReceiver> messageReceiverClassMap = new HashMap<String, MessageReceiver>(1);
    messageReceiverClassMap.put(WSDL2Constants.MEP_URI_IN_OUT, new MissedCallServiceMessageReceiverInOut());
    AxisService missedCallsService = AxisService.createService(MissedCallServiceSkeleton.class.getName(), configContext.getAxisConfiguration(), messageReceiverClassMap, "", "http://tempuri.org/", configContext.getAxisConfiguration().getSystemClassLoader());    
    missedCallsService.setEndpointName("MissedCalls");
    missedCallsService.setName("MissedCalls");

    configContext.getAxisConfiguration().addService(missedCallsService);

  }

  public void start() throws AxisFault {
    super.start();
  }
}
