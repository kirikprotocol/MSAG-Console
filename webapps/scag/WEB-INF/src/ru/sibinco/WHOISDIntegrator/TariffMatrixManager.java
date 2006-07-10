package ru.sibinco.WHOISDIntegrator;

import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.StatusDisconnectedException;
import ru.sibinco.scag.backend.Manager;
import ru.sibinco.scag.backend.SCAGAppContext;

import javax.servlet.http.HttpServletRequest;

/**
 * Created by IntelliJ IDEA.
 * User: dym
 * Date: 06.07.2006
 * Time: 12:22:34
 * To change this template use File | Settings | File Templates.
 */
public class TariffMatrixManager extends Manager {
    public static final String WHOISD_ERROR_PREFIX = "Couldn't reload tariff matrix";
    private String pathToWrite;
    private HttpServletRequest req;
    private boolean isMultipartFormat;

    public TariffMatrixManager() {

    }

    public synchronized void applyTariffMatrix(final String pathToWrite, final HttpServletRequest req, final boolean isMultipartFormat, SCAGAppContext appContext) throws Exception {
      this.pathToWrite = pathToWrite;
      this.req = req;
      this.isMultipartFormat = isMultipartFormat;
      try {
        appContext.getScag().invokeCommand("reloadTariffMatrix",null,appContext,this,pathToWrite);
      } catch (SibincoException se) {
        if (!(se instanceof StatusDisconnectedException))
          if (se.getMessage().startsWith(WHOISD_ERROR_PREFIX))
            throw new WHOISDException(se.getMessage());
          else {
            if (se.getCause()!=null) throw (Exception)se.getCause();
            throw se;
          }
       }
    }

    public void store() throws SibincoException {
      try {
        WHOISDServlet.saveFile(pathToWrite, req, isMultipartFormat);
      } catch (Exception e) {
        //logger.error("Couldn't save tariff matrix config", e);
        throw new SibincoException("Couldn't save tariff matrix config", e);
      }
    }

    public void parse() throws Throwable {
      SAXParserImpl.parseTariffMatrix(pathToWrite);
    }
}
