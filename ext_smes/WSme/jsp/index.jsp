<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ include file="inc/header.jsp"%>
<%@ page import="ru.novosoft.smsc.wsme.*,
                 ru.novosoft.smsc.wsme.beans.*,
                 ru.novosoft.smsc.jsp.SMSCErrors,
                 ru.novosoft.smsc.jsp.SMSCJspException,
                 java.util.List,
                 ru.novosoft.smsc.util.StringEncoderDecoder,
                 java.util.Iterator"%>
<jsp:useBean id="bean" scope="page" class="ru.novosoft.smsc.wsme.beans.WSmeFormBean" />
<jsp:setProperty name="bean" property="*"/>
<%
  TITLE="Welcome SME Administration";
  MENU0_SELECTION = "MENU0_SERVICES";
  //MENU1_SELECTION = "WSME_INDEX";

  int beanResult = bean.RESULT_OK;
  switch(beanResult = bean.process(request, errorMessages))
  {
    case WSmeFormBean.RESULT_VISITORS:
      response.sendRedirect("visitors.jsp");
      return;
    case WSmeFormBean.RESULT_LANGS:
      response.sendRedirect("langs.jsp");
      return;
    case WSmeFormBean.RESULT_ADS:
      response.sendRedirect("ads.jsp");
      return;
    case WSmeFormBean.RESULT_DONE:
      STATUS.append("Ok");
      break;
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
<br><%@ include file="inc/menu.jsp"%>
<script>
function showhide(elem)
{
	var st = elem.nextSibling.style;
	if (st.display != "none")
	{
		elem.className = "secTitleClosed";
		//elem.style.borderBottom = "1px solid #888888";
		st.display="none";
	}
	else
	{
		elem.className = "secTitleOpened";
		st.display = "";
	}
}
</script>
<dl><%-- ############################ StartupLoader ########################### --%>
<dt class=secTitleClosed onclick='showhide(this)'>StartupLoader</dt>
<dd style="display:none">
	<dl>
	<dt class=secTitleClosed onclick='showhide(this)'>DataSourceDrivers</dt>
	<dd style="display:none">
	<%
    for (Iterator i = bean.getDatasourceDrivers().iterator(); i.hasNext();)
    {
      String name = (String) i.next();
			%>
		<dl>
		<dt class=secTitleClosed onclick='showhide(this)'><%=name%></dt>
		<dd style="display:none">
			<table class=secRep cellspacing=1 width="100%">
			<%row=0;%>
			<col width=180px>
			<tr class=row<%=(row++)&1%>>
				<th class=label>type:</th>
				<td><input class=txtW name="StartupLoader.DataSourceDrivers.<%=StringEncoderDecoder.encode(name)%>.type" value="<%=StringEncoderDecoder.encode(bean.getStringParam("StartupLoader.DataSourceDrivers." + name + ".type"))%>"></td>
			</tr>
			<tr class=row<%=(row++)&1%>>
				<th class=label>loadup:</th>
				<td><input class=txtW name="StartupLoader.DataSourceDrivers.<%=StringEncoderDecoder.encode(name)%>.loadup" value="<%=StringEncoderDecoder.encode(bean.getStringParam("StartupLoader.DataSourceDrivers." + name + ".loadup"))%>"></td>
			</tr>
			</table>
		</dd>
		</dl><%
    }%>
	</dd>
	</dl>
</dd>
</dl>
<dl><%-- ############################ WSme ########################### --%>
<dt class=secTitleOpened onclick='showhide(this)'>WSme</dt>
<dd style="display">
<%row=0;%>
<table class=secRep cellspacing=1 width="100%">
<col width=180px>
<tr class=row<%=(row++)&1%>>
	<th class=label>SvcType:</th>
	<td><input class=txtW name="WSme.SvcType" value="<%=StringEncoderDecoder.encode(bean.getStringParam("WSme.SvcType"))%>"></td>
</tr>
<tr class=row<%=(row++)&1%>>
	<th class=label>ProtocolId:</th>
	<td><input class=txtW name="WSme.ProtocolId" value="<%=bean.getIntParam("WSme.ProtocolId")%>"></td>
</tr>
</table>
<dl><%-- ############################ WSme.SMSC ########################### --%>
<dt class=secTitleClosed onclick='showhide(this)'>SMSC</dt>
<dd style="display:none">
<table class=secRep cellspacing=1 width="100%">
<%row=0;%>
<col width=180px>
<tr class=row<%=(row++)&1%>>
	<th class=label>host:</th>
	<td><input class=txtW name="WSme.SMSC.host" value="<%=StringEncoderDecoder.encode(bean.getStringParam("WSme.SMSC.host"))%>"></td>
</tr>
<tr class=row<%=(row++)&1%>>
	<th class=label>port:</th>
	<td><input class=txtW name="WSme.SMSC.port" value="<%=bean.getIntParam("WSme.SMSC.port")%>"></td>
</tr>
<tr class=row<%=(row++)&1%>>
	<th class=label>sid:</th>
	<td><input class=txtW name="WSme.SMSC.sid" value="<%=StringEncoderDecoder.encode(bean.getStringParam("WSme.SMSC.sid"))%>"></td>
</tr>
<tr class=row<%=(row++)&1%>>
	<th class=label>timeout:</th>
	<td><input class=txtW name="WSme.SMSC.timeout" value="<%=bean.getIntParam("WSme.SMSC.timeout")%>"></td>
</tr>
<tr class=row<%=(row++)&1%>>
	<th class=label>password:</th>
	<td><input class=txtW name="WSme.SMSC.password" value="<%=StringEncoderDecoder.encode(bean.getStringParam("WSme.SMSC.password"))%>"></td>
</tr>
</table>
</dd>
</dl>
<dl><%-- ############################ WSme.ThreadPool ########################### --%>
<dt class=secTitleClosed onclick='showhide(this)'>ThreadPool</dt>
<dd style="display:none">
<%row=0;%>
<table class=secRep cellspacing=1 width="100%">
<col width=180px>
<tr class=row<%=(row++)&1%>>
	<th class=label>max:</th>
	<td><input class=txtW name="WSme.ThreadPool.max" value="<%=bean.getIntParam("WSme.ThreadPool.max")%>"></td>
</tr>
<tr class=row<%=(row++)&1%>>
	<th class=label>ProtocolId:</th>
	<td><input class=txtW name="WSme.ThreadPool.init" value="<%=bean.getIntParam("WSme.ThreadPool.init")%>"></td>
</tr>
</table>
</dd>
</dl>
<dl><%-- ############################ WSme.DataSource ########################### --%>
<dt class=secTitleClosed onclick='showhide(this)'>DataSource</dt>
<dd style="display:none">
<table class=secRep cellspacing=1 width="100%">
<%row=0;%>
<col width=180px>
<tr class=row<%=(row++)&1%>>
	<th class=label>type:</th>
	<td><input class=txtW name="WSme.DataSource.type" value="<%=StringEncoderDecoder.encode(bean.getStringParam("WSme.DataSource.type"))%>"></td>
</tr>
<tr class=row<%=(row++)&1%>>
	<th class=label>connections:</th>
	<td><input class=txtW name="WSme.DataSource.connections" value="<%=bean.getIntParam("WSme.DataSource.connections")%>"></td>
</tr>
<tr class=row<%=(row++)&1%>>
	<th class=label>dbInstance:</th>
	<td><input class=txtW name="WSme.DataSource.dbInstance" value="<%=StringEncoderDecoder.encode(bean.getStringParam("WSme.DataSource.dbInstance"))%>"></td>
</tr>
<tr class=row<%=(row++)&1%>>
	<th class=label>dbUserName:</th>
	<td><input class=txtW name="WSme.DataSource.dbUserName" value="<%=StringEncoderDecoder.encode(bean.getStringParam("WSme.DataSource.dbUserName"))%>"></td>
</tr>
<tr class=row<%=(row++)&1%>>
	<th class=label>dbUserPassword:</th>
	<td><input class=txtW name="WSme.DataSource.dbUserPassword" value="<%=StringEncoderDecoder.encode(bean.getStringParam("WSme.DataSource.dbUserPassword"))%>"></td>
</tr>
</table>
</dd>
</dl>
<dl><%-- ############################ WSme.Admin ########################### --%>
<dt class=secTitleClosed onclick='showhide(this)'>Admin</dt>
<dd style="display:none">
<table class=secRep cellspacing=1 width="100%">
<%row=0;%>
<col width=180px>
<tr class=row<%=(row++)&1%>>
	<th class=label>host:</th>
	<td><input class=txtW name="WSme.Admin.host" value="<%=StringEncoderDecoder.encode(bean.getStringParam("WSme.Admin.host"))%>"></td>
</tr>
<tr class=row<%=(row++)&1%>>
	<th class=label>port:</th>
	<td><input class=txtW name="WSme.Admin.port" value="<%=bean.getIntParam("WSme.Admin.port")%>"></td>
</tr>
</table>
<dl><%-- ############################ WSme.Admin.Web ########################### --%>
<dt class=secTitleClosed onclick='showhide(this)'>Web</dt>
<dd style="display:none">
<dl><%-- ############################ WSme.Admin.Web.jdbc ########################### --%>
<dt class=secTitleOpened onclick='showhide(this)'>jdbc</dt>
<dd style="display">
<table class=secRep cellspacing=1 width="100%">
<%row=0;%>
<col width=180px>
<tr class=row<%=(row++)&1%>>
	<th class=label>source:</th>
	<td><input class=txtW name="WSme.Admin.Web.jdbc.source" value="<%=StringEncoderDecoder.encode(bean.getStringParam("WSme.Admin.Web.jdbc.source"))%>"></td>
</tr>
<tr class=row<%=(row++)&1%>>
	<th class=label>driver:</th>
	<td><input class=txtW name="WSme.Admin.Web.jdbc.driver" value="<%=StringEncoderDecoder.encode(bean.getStringParam("WSme.Admin.Web.jdbc.driver"))%>"></td>
</tr>
<tr class=row<%=(row++)&1%>>
	<th class=label>user:</th>
	<td><input class=txtW name="WSme.Admin.Web.jdbc.user" value="<%=StringEncoderDecoder.encode(bean.getStringParam("WSme.Admin.Web.jdbc.user"))%>"></td>
</tr>
<tr class=row<%=(row++)&1%>>
	<th class=label>password:</th>
	<td><input class=txtW name="WSme.Admin.Web.jdbc.password" value="<%=StringEncoderDecoder.encode(bean.getStringParam("WSme.Admin.Web.jdbc.password"))%>"></td>
</tr>
</table>
</dd></dl></dd></dl>
</dd>
</dl>
<dl><%-- ############################ WSme.LangManager ########################### --%>
<dt class=secTitleClosed onclick='showhide(this)'>LangManager</dt>
<dd style="display:none">
<table class=secRep cellspacing=1 width="100%">
<%row=0;%>
<col width=180px>
<tr class=row<%=(row++)&1%>>
	<th class=label>defaultLang:</th>
	<td><input class=txtW name="WSme.LangManager.defaultLang" value="<%=StringEncoderDecoder.encode(bean.getStringParam("WSme.LangManager.defaultLang"))%>"></td>
</tr>
</table>
</dd>
</dl>
<dl><%-- ############################ WSme.AdManager ########################### --%>
<dt class=secTitleClosed onclick='showhide(this)'>AdManager</dt>
<dd style="display:none">
<dl><%-- ############################ WSme.AdManager.History ########################### --%>
<dt class=secTitleOpened onclick='showhide(this)'>History</dt>
<dd style="display">
<table class=secRep cellspacing=1 width="100%">
<%row=0;%>
<col width=180px>
<tr class=row<%=(row++)&1%>>
	<th class=label>age (days):</th>
	<td><input class=txtW name="WSme.AdManager.History.age" value="<%=bean.getIntParam("WSme.AdManager.History.age")%>"></td>
</tr>
<tr class=row<%=(row++)&1%>>
	<th class=label>cleanup (seconds):</th>
	<td><input class=txtW name="WSme.AdManager.History.cleanup" value="<%=bean.getIntParam("WSme.AdManager.History.cleanup")%>"></td>
</tr>
<tr class=row<%=(row++)&1%>>
	<th class=label>message life (seconds):</th>
	<td><input class=txtW name="WSme.AdManager.History.messageLife" value="<%=bean.getIntParam("WSme.AdManager.History.messageLife")%>"></td>
</tr>
</table>
</dd></dl>
</dd>
</dl>
</dd>
</dl>

<div class=secButtons>
<input class=btn type=submit name=btnApply value="Apply" title="Apply">
<input class=btn type=submit name=btnCancel value="Cancel" title="Cancel">
</div>

<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>