package ru.sibinco.scag.beans.rules;

import ru.sibinco.scag.backend.transport.Transport;
import ru.sibinco.scag.beans.SCAGBean;
import ru.sibinco.scag.beans.SCAGJspException;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

/**
 * Created by IntelliJ IDEA.
 * User: dym
 * Date: 04.10.2006
 * Time: 15:26:23
 * To change this template use File | Settings | File Templates.
 */
public class Rules extends SCAGBean {
  private String id;
    
  public void setId(String id) {
    this.id = id;
  }

  public String getId() {
    return id;
  }

  public void process(final HttpServletRequest request, final HttpServletResponse response) throws SCAGJspException {
    super.process(request,response);
  }

  public RuleState getSmppRuleState() {
      return (appContext.getRuleManager().getRuleState(id, Transport.SMPP_TRANSPORT_NAME));
  }

  public RuleState getHttpRuleState() {
      return (appContext.getRuleManager().getRuleState(id, Transport.HTTP_TRANSPORT_NAME));
  }

  public RuleState getMmsRuleState() {
      return (appContext.getRuleManager().getRuleState(id, Transport.MMS_TRANSPORT_NAME));
  }

}
