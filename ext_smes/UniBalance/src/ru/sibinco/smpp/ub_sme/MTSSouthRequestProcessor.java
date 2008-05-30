package ru.sibinco.smpp.ub_sme;

import ru.sibinco.smpp.ub_sme.mg.MGManager;
import ru.sibinco.util.threads.ThreadsPool;
import ru.aurorisoft.smpp.Message;
import ru.aurorisoft.smpp.PDU;

import java.util.Properties;

import com.logica.smpp.Data;
import org.apache.log4j.Category;

/**
 * User: pasha
 * Date: 14.01.2008
 * Time: 15:47:44
 */
public class MTSSouthRequestProcessor implements RequestProcessor {
  private final static Category logger = Category.getInstance(MTSSouthRequestProcessor.class);
  private SmeEngine smeEngine;
  private ThreadsPool threadsPool;
  private MGManager mgManager;
  private BannerManager bannerManager;

  public void init(Properties config) throws InitializationException {
    mgManager = new MGManager(threadsPool, smeEngine);
    bannerManager = new BannerManager(threadsPool, smeEngine);
  }

  public void setSmeEngine(SmeEngine smeEngine) {
    this.smeEngine = smeEngine;
    if(mgManager !=null){
      mgManager.setSmeEngine(smeEngine);
    }
  }

  public void setThreadsPool(ThreadsPool threadsPool) {
    this.threadsPool = threadsPool;
    if(bannerManager!=null){
      bannerManager.setThreadsPool(threadsPool);
    }
  }

  /*
    Здесь происходит обработка входящего сообщения.
    Если пришел запрос от абонента то стартуется state машина.
    Если пришел ответ от MG, то передаем соотвествующему MGState баланс.
  */
  public void processIncomingMessage(Message message, long abonentRequestTime) {
    if (message.getSourceAddress().equals(mgManager.getMgAddress())) { // MG response
      if (logger.isDebugEnabled())
        logger.debug("Got MG response from " + message.getSourceAddress());
      mgManager.handleMGResponse(message);
    } else { // abonent request
      if (smeEngine.getRequestCounter() != null) {
        smeEngine.getRequestCounter().count();
      }
      smeEngine.sendDeliverSmResponse(message, Data.ESME_ROK);
      State state = new MTSSouthState(message, mgManager, bannerManager);
      state.startProcessing(threadsPool, smeEngine);
    }
  }

  /*
    Обрабатвается PDU запроса в MG, если у PDU cтатус ошибки, то соотв. MgState закрывается.
  */
  public void handleErrorPDU(PDU pdu) {
    mgManager.handleErrorPDU(pdu);
  }

}
