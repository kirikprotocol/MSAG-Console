<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ include file="inc/header.jsp"%>
<%@ page import="ru.novosoft.smsc.wsme.*,
                 ru.novosoft.smsc.wsme.beans.*,
                 ru.novosoft.smsc.jsp.SMSCErrors,
                 ru.novosoft.smsc.jsp.SMSCJspException,
                 java.util.List,
                 ru.novosoft.smsc.util.StringEncoderDecoder,
                 java.util.Iterator,
                 ru.novosoft.smsc.admin.Constants"%>
<jsp:useBean id="bean" scope="page" class="ru.novosoft.smsc.wsme.beans.WSmeFormBean" />
<jsp:setProperty name="bean" property="*"/>
<%
  ServiceIDForShowStatus = Constants.WSME_SME_ID;
  TITLE="Welcome SME Administration";
  MENU0_SELECTION = "MENU0_SERVICES";
  //MENU1_SELECTION = "WSME_INDEX";

  int beanResult = bean.RESULT_OK;
  switch(beanResult = bean.process(request, errorMessages, loginedUserPrincipal))
  {
    case WSmeFormBean.RESULT_VISITORS:
      response.sendRedirect("visitors.jsp");
      return;
    case WSmeFormBean.RESULT_LANGS:
      response.sendRedirect("langs.jsp");
      return;
    case WSmeFormBean.RESULT_DONE:
      response.sendRedirect("index.jsp");
      return;
    case WSmeFormBean.RESULT_ADS:
      response.sendRedirect("ads.jsp");
      return;
    case WSmeFormBean.RESULT_OK:
      STATUS.append("Ok");
      break;
    case WSmeFormBean.RESULT_ERROR:
      STATUS.append("<span class=CF00>Error</span>");
      break;
    default:
      STATUS.append("<span class=CF00>Error "+beanResult+"</span>");
      errorMessages.add(new SMSCJspException(SMSCErrors.error.services.unknownAction));
  }
  int row = 0;
%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="inc/start_stop.jsp"%>
<%@ include file="/WEB-INF/inc/collapsing_tree.jsp"%>
<%@ include file="inc/menu.jsp"%>
<div class=content>
<%
	//################################## StartupLoader #############################
	startSection(out, "StartupLoader", "Startup Loader", false);
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~ StartupLoader.DataSourceDrivers ~~~~~~~~~~~~~~~~~~~~~~~~~
		startSection(out, "StartupLoader.DataSourceDrivers", "Data Source Drivers", false);
			for (Iterator i = bean.getDatasourceDrivers().iterator(); i.hasNext();)
			{
				String name = (String) i.next();
				String encName = StringEncoderDecoder.encode(name);
				//---------------------------------- StartupLoader.DataSourceDrivers.%name% -----------------------------
				startSection(out, "StartupLoader.DataSourceDrivers." + encName, encName, false);
					startParams(out);
						param(out, "type",   "StartupLoader.DataSourceDrivers." + encName + ".type",   bean.getStringParam("StartupLoader.DataSourceDrivers." + name + ".type"));
						param(out, "loadup", "StartupLoader.DataSourceDrivers." + encName + ".loadup", bean.getStringParam("StartupLoader.DataSourceDrivers." + name + ".loadup"));
					finishParams(out);
				finishSection(out);
			}
		finishSection(out);
	finishSection(out);
	//################################## WSme #############################
	startSection(out, "WSme", "WSme", true);
		startParams(out);
			param(out, "service type", "WSme.SvcType",    bean.getStringParam("WSme.SvcType"));
			param(out, "protocol ID",  "WSme.ProtocolId", bean.getIntParam("WSme.ProtocolId"));
		finishParams(out);
		startSection(out, "WSme.SMSC", "SMSC", false);
			startParams(out);
				param(out, "host", "WSme.SMSC.host",    bean.getStringParam("WSme.SMSC.host"));
				param(out, "port", "WSme.SMSC.port",    bean.getIntParam("WSme.SMSC.port"));
				param(out, "system ID", "WSme.SMSC.sid",    bean.getStringParam("WSme.SMSC.sid"));
				param(out, "timeout", "WSme.SMSC.timeout",    bean.getIntParam("WSme.SMSC.timeout"));
				param(out, "password", "WSme.SMSC.password",    bean.getStringParam("WSme.SMSC.password"));
			finishParams(out);
		finishSection(out);
		startSection(out, "WSme.ThreadPool", "Thread Pool", false);
			startParams(out);
				param(out, "maximum", "WSme.ThreadPool.max",    bean.getIntParam("WSme.ThreadPool.max"));
				param(out, "initial", "WSme.ThreadPool.init",   bean.getIntParam("WSme.ThreadPool.init"));
			finishParams(out);
		finishSection(out);
		startSection(out, "WSme.DataSource", "Data Source", false);
			startParams(out);
				param(out, "type",           "WSme.DataSource.type",           bean.getStringParam("WSme.DataSource.type"));
				param(out, "connections",    "WSme.DataSource.connections",    bean.getIntParam("WSme.DataSource.connections"));
				param(out, "dbInstance",     "WSme.DataSource.dbInstance",     bean.getStringParam("WSme.DataSource.dbInstance"));
				param(out, "dbUserName",     "WSme.DataSource.dbUserName",     bean.getStringParam("WSme.DataSource.dbUserName"));
				param(out, "dbUserPassword", "WSme.DataSource.dbUserPassword", bean.getStringParam("WSme.DataSource.dbUserPassword"));
			finishParams(out);
		finishSection(out);
		startSection(out, "WSme.Admin", "Admin", false);
			startParams(out);
				param(out, "host", "WSme.Admin.host",    bean.getStringParam("WSme.Admin.host"));
				param(out, "port", "WSme.Admin.port",    bean.getIntParam("WSme.Admin.port"));
			finishParams(out);
			startSection(out, "WSme.Admin.Web", "Web", false);
				startSection(out, "WSme.Admin.Web.jdbc", "JDBC", true);
					startParams(out);
						param(out, "source",   "WSme.Admin.Web.jdbc.source",   bean.getStringParam("WSme.Admin.Web.jdbc.source"));
						param(out, "driver",   "WSme.Admin.Web.jdbc.driver",   bean.getStringParam("WSme.Admin.Web.jdbc.driver"));
						param(out, "user",     "WSme.Admin.Web.jdbc.user",     bean.getStringParam("WSme.Admin.Web.jdbc.user"));
						param(out, "password", "WSme.Admin.Web.jdbc.password", bean.getStringParam("WSme.Admin.Web.jdbc.password"));
					finishParams(out);
				finishSection(out);
			finishSection(out);
		finishSection(out);
		startSection(out, "WSme.LangManager", "Languages Manager", false);
			startParams(out);
				param(out, "default language", "WSme.LangManager.defaultLang",    bean.getStringParam("WSme.LangManager.defaultLang"));
			finishParams(out);
		finishSection(out);
		startSection(out, "WSme.AdManager", "Advertise Messages Manager", false);
			startSection(out, "WSme.AdManager.History", "History", true);
				startParams(out);
					param(out, "age (in days)",                  "WSme.AdManager.History.age",         bean.getIntParam("WSme.AdManager.History.age"));
					param(out, "cleanup timeout (in seconds)",   "WSme.AdManager.History.cleanup",     bean.getIntParam("WSme.AdManager.History.cleanup"));
					param(out, "message life time (in seconds)", "WSme.AdManager.History.messageLife", bean.getIntParam("WSme.AdManager.History.messageLife"));
				finishParams(out);
			finishSection(out);
		finishSection(out);
	finishSection(out);
%></div><%
page_menu_begin(out);
page_menu_button(out, "btnApply",  "Apply",  "Apply changes");
page_menu_button(out, "btnCancel", "Cancel", "Cancel changes");
page_menu_space(out);
page_menu_end(out);
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>