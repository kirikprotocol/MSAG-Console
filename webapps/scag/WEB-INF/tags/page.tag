<%@
 taglib uri="http://java.sun.com/jsp/jstl/core" prefix="c"%><%@
 taglib uri="http://java.sun.com/jsp/jstl/fmt" prefix="fmt"%>
<%@
 taglib prefix="sm" tagdir="/WEB-INF/tags"%><%@
 taglib prefix="sm-mm" tagdir="/WEB-INF/tags/main_menu"%><%@

 tag body-content="scriptless"                  %><%@
 attribute name="beanClass"    required="false" %><%@
 attribute name="title"        required="true"  %><%@
 attribute name="menu"         required="false" %><%@
 attribute name="menu2"        required="false" %><%@
 attribute name="form_method"  required="false" %><%@
 attribute name="form_uri"     required="false" %><%@
 attribute name="form_enctype" required="false" %><%@
 attribute name="onLoad"       required="false" %><%@
 attribute name="rawBody"      required="false" %>
<%! private java.util.List errorMessages = new java.util.ArrayList();%>
<%
errorMessages.clear();
request.setAttribute(ru.sibinco.scag.Constants.SCAG_ERROR_MESSAGES_ATTRIBUTE_NAME, errorMessages);
%><%
  Object beanClassObj = jspContext.getAttribute("beanClass");
  if (beanClassObj == null || (beanClassObj instanceof String && ((String)beanClassObj).trim().length() == 0)) //!pageContext
  {
    final StringBuffer buffer = new StringBuffer();
    final String servletPath = request.getServletPath();
    int pos = servletPath.indexOf(".jsp");
    if (pos > 0)
      buffer.append(servletPath.substring(0, pos));
    else
      if (servletPath.endsWith("/"))
        buffer.append(servletPath.substring(0, servletPath.length()-1));
      else
        buffer.append(servletPath);

    for (int i = 0; i < buffer.length(); i++)
      if (buffer.charAt(i) == '/')
        buffer.setCharAt(i, '.');

    final int cnp = buffer.lastIndexOf(".")+1;
    buffer.setCharAt(cnp, Character.toUpperCase(buffer.charAt(cnp)));

    final String generatedBeanClass = buffer.substring(1);
    try {
      Thread.currentThread().getContextClassLoader().loadClass("ru.sibinco.scag.beans." + generatedBeanClass);
      jspContext.setAttribute("generatedBeanClass", generatedBeanClass); //!pageContext
    } catch(ClassNotFoundException e) {
      //skip it
    }
  }

%>
<c:if test="${!empty param.locale}">
  <fmt:setLocale value="${param.locale}" scope="session"/>
</c:if>
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<html>
<head>
  <title><fmt:message>common.title</fmt:message><c:if test="${!empty title}">: <fmt:message>${title}<c:if test="${!empty param.editId}"><fmt:param value="${param.editId}"/></c:if></fmt:message></c:if></title>
  <base href="<%=request.getScheme()%>://<%=request.getServerName()%>:<%=request.getServerPort()+request.getContextPath()%>/">
  <link rel="STYLESHEET" type="text/css" href="content/styles/colors.css">
  <link rel="STYLESHEET" type="text/css" href="content/styles/common.css">
  <link rel="STYLESHEET" type="text/css" href="content/styles/styles.jsp">
  <link rel="STYLESHEET" type="text/css" href="content/styles/list.css">
  <link rel="STYLESHEET" type="text/css" href="content/styles/properties_list.css">
  <link rel="STYLESHEET" type="text/css" href="content/styles/main_menu.css">
<%--  <link rel="STYLESHEET" type="text/css" href="content/styles/collapsing_tree.css">--%>
  <link rel="STYLESHEET" type="text/css" href="content/styles/collapsing_list.css">
<%--  <link rel="STYLESHEET" type="text/css" href="content/styles/navbar.css">--%>
  <link rel="STYLESHEET" type="text/css" href="content/styles/messages.css">
  <link rel="STYLESHEET" type="text/css" href="content/styles/page_menu.css">
  <link rel="STYLESHEET" type="text/css" href="content/styles/sme_menu.css">
  <link rel="STYLESHEET" type="text/css" href="content/styles/calendar.css">

<%--  <link rel="STYLESHEET" type="text/css" href="content/styles/collapsing_tree_dl.css">--%>
  <script src="content/scripts/scripts.jsp" type="text/javascript"></script>
  <script src="content/scripts/mootools.js" type="text/javascript"></script>
  <script src="content/scripts/datasource.js" type="text/javascript"></script>
</head>
<body onload="initjsDOMenu(); assignOpener(); refreshSCAGStatusObject(); ${onLoad} " onKeyPress="return disableCtrlKeyCombination(event,'${param.editId}');" onKeyDown="return disableCtrlKeyCombination(event,'${param.editId}');">
  <!--calendar-->
  <iframe id=calendarIFrame class=calendarHiddenLayer2 src="content/images/blank.html"></iframe>
  <div id=calendarPanel class=calendarHiddenLayer
    oncontextmenu="return false;"
    onselectstart="return false;"
    ondragstart="return false;"
    onmousedown="return calendarMD(event);"
    onmouseup="return calendarMU(event);"
    onmousemove="return calendarMM(event);"
    onlosecapture="return calendarClose();"
    onfocusout="return calendarClose();">
  <table border=1 bgcolor="#dododo" cellpadding=0 cellspacing=2>
  <tr>
  <td style="border:0;">
    <table cellpadding=1 cellspacing=0>
      <tr>
        <td style="border:0; border-bottom:solid 1px #000000; padding-bottom:2px;padding-top:0px">
          <table height=100% cellpadding=0 cellspacing=0>
            <tr align=center class=calendarYear>
              <td colspan=5 id=calendarYearPanel></td>
              <td colspan=2 align=rigth>
                <table cellpadding=0 cellspacing=0 class=calendarMonth>
                  <tr align=right>
                    <td width="100%" name=calendarMonthMinus><</td>
                    <td name=calendarMonthPlus >></td>
                  </tr>
                </table>
              </td>
            </tr>
            <tr align=center style="padding-top:2px" class=calendarWeekDay id=calendarWeekDays></tr>
          </table>
        </td>
      </tr>
      <tr>
        <td id=calendarDaysPanel style="border:0; border-top:solid 1px #ffffff; padding-top:2px">
          <table height=100% cellpadding=0 cellspacing=0 class=calendarDate id=calendarDaysTable></table>
        </td>
      </tr>
      <tr>
        <td id=calendarTimePanel style="border-top:solid 1px #ffffff;display:none;padding-top:2px;">
          <table cellpadding=0 cellspacing=0>
          <tr>
            <td rowspan=2 width=2px></td>
            <td rowspan=2><div class=calendarTimeH id=calendarTimeHour>00</div></td>
            <td valign=bottom align=center>
              <div name=calendarTimeHourUp style="background-color:#f0f0f0;border-top:solid 1px #000000;width:9px;height:6px;font-size:7px">&#9650</div>
            </td>
            <td rowspan=2><div class=calendarTimeM id=calendarTimeMinute>00</div></td>
            <td valign=bottom align=center>
              <div name=calendarTimeMinuteUp style="background-color:#f0f0f0;border-top:solid 1px #000000;width:9px;height:6px;font-size:7px">&#9650</div>
            </td>
            <td rowspan=2><div class=calendarTimeS id=calendarTimeSecond>00</div></td>
            <td valign=bottom align=center>
              <div name=calendarTimeSecondUp style="background-color:#f0f0f0;border-top:solid 1px #000000;border-right:solid 1px #000000;width:9px;height:6px;font-size:7px">&#9650</div>
            </td>
            <td rowspan=2 width=30px align=right>
              <div id=calendarAMPMsv name=calendarAMPMsv align=center style="padding:0px; padding-left:2px; background-color:#f0f0f0;border:solid 1px #000000;width:24px;height:16px;font-family:Tahoma,Arial,Helvetica,sans-serif;font-size:11px;font-weight:bold;visibility: hidden;">PM</div>
            </td>
            <td rowspan=2 width=40px align=right>
              <div name=calendarTimeOk align=center style="padding:0px; background-color:#f0f0f0;border:solid 1px #000000;width:30px;height:16px;font-family:Tahoma,Arial,Helvetica,sans-serif;font-size:11px;font-weight:bold;">Ok</div>
            </td>
          </tr>
          <tr>
            <td align=center>
              <div name=calendarTimeHourDown style="background-color:#f0f0f0;border-bottom:solid 1px #000000;width:9px;height:6px;font-size:7px">&#9660</div>
            </td>
            <td align=center>
              <div name=calendarTimeMinuteDown style="background-color:#f0f0f0;border-bottom:solid 1px #000000;width:9px;height:6px;font-size:7px">&#9660</div>
            </td>
            <td align=center>
              <div name=calendarTimeSecondDown style="background-color:#f0f0f0;border-bottom:solid 1px #000000;border-right:solid 1px #000000;width:9px;height:6px;font-size:7px">&#9660</div>
            </td>
          </tr>
          </table>
        </td>
      </tr>
    </table>
  </td>
  </tr>
  </table>
  </div>
<!--end of calendar-->

<%if (request.getUserPrincipal() != null) {%>
  <script >
  var serviceStatusDataSource = new StringTableDataSource({url: '<%=request.getContextPath()%>/gw/status/status.jsp'});
  var observer = new ElementObserver({elementId: 'SCAGStatusSpan', field: 'status' });
  serviceStatusDataSource.addObserver(observer);
  </script>
<%}%>
  <table height="100%" cellspacing=0 cellpadding=0 class=main_table>
  <tr>
    <td width=50 background="content/images/smsc_02.jpg" rowspan=3></td>
    <td width=30 background="content/images/smsc_03.jpg" rowspan=3></td>
    <td bgcolor="#F3F0E3" style="position: relative; top:0; left:0;">
      <!-- background -->
      <table cellspacing=0 cellpadding=0 height="100%" style="position: absolute; top:0; left:0; z-index: -1;">
      <tr>
        <td width=12px background="content/images/smsc_24.jpg"></td>
        <td>&nbsp;</td>
        <td width=12px background="content/images/smsc_26.jpg"></td>
      </tr>
      </table>
      <!-- "SMS Service Center" header -->
      <table cellspacing=0 cellpadding=0>
      <tr>
        <td width=380><IMG SRC="content/images/smsc_04.jpg" HEIGHT=45 width=380 ALT=""></td>
        <td bgcolor="#161E49"><span style="color: #6F77A3; font-size: 11px; font-weight:bold; vertical-align: -10px;">Version&nbsp;2.0</span></td>
        <td width=12 background="content/images/smsc_06.jpg"></td>
      </tr>
      </table>
      <!-- main menu -->
      <sm-mm:top_menu/>

      <table width="100%" id="MAIN_MENU_TABLE" border="0"  cellspacing="0" cellpadding="0" class="main_menu">
		 <tr>
			<td background="content/images/smsc_13.jpg" width="26"></td>
			<td background="content/images/smsc_14.jpg" width="100%">&nbsp;</td>
      <%
        java.util.ArrayList localesList = (java.util.ArrayList)application.getAttribute("localesList");
        for (int i=0;i<localesList.size();i++) {
          ru.sibinco.scag.util.LocaleManager.SCAGLocale locale = (ru.sibinco.scag.util.LocaleManager.SCAGLocale)localesList.get(i);
          out.println("<td background=\"content/images/smsc_14.jpg\"><a href=\"?locale="+locale.locale.getLanguage()+"\"><img src=\"content/images/"+locale.icon+"\" alt="+locale.locale.getDisplayName(ru.sibinco.scag.util.LocaleMessages.getInstance().getLocale(session))+" class=langicon onmouseover=\"return changeIcon(event);\" onmouseout=\"return changeIcon(event);\"></a></td>");
        }
      %>
            <%
                if (request != null && request.getUserPrincipal() != null) {
            %>
                   <td background="content/images/smsc_14.jpg"><a ID="MENU0_LOGOUT" href="logout.jsp"><fmt:message>common.logout</fmt:message>&nbsp;"<%=request.getUserPrincipal().getName()%>"</a></td>
            <%
                }
            %>
			<td background="content/images/smsc_16.jpg" width="52"></td>
		 </tr>
		</table>
		<span id="MENU0_NONE"></span>
        <table cellpadding=0 cellspacing=0 height=30px class=smsc_status>
           <tr>
             <th background="content/images/smsc_17.jpg" nowrap><c:if test="${!empty title}"><fmt:message>${title}<c:if test="${!empty param.editId}"><fmt:param value="${param.editId}"/></c:if></fmt:message></c:if></th>
             <td align="right">&nbsp;
                <%if (request.getUserPrincipal() != null) {%>
                    <span id="SCAGStatusSpan"></span>
                <%}%>
             </td>
             <td width=12px background="content/images/smsc_19.jpg" style="padding-right:0px;"></td>
           </tr>
        </table>
        <form name="opForm" id=opForm
                <c:if test="${!empty form_uri}">action="${form_uri}"</c:if>
                method="<c:choose><c:when test="${!empty form_method}">${form_method}</c:when><c:otherwise>post</c:otherwise></c:choose>"
                <c:if test="${!empty form_method}">method="${form_method}"</c:if>
                <c:if test="${!empty form_enctype}">enctype="${form_enctype}"</c:if>
                onSubmit="return validateForm(this)">
                <input type=hidden ID=jbutton value=jbutton>
<%--                <input type=hidden name=startPosition ID=startPosition value="">--%>

        <c:choose>
          <c:when test="${!empty beanClass}"><sm:bean className="ru.sibinco.scag.beans.${beanClass}"/></c:when>
          <c:otherwise><c:if test="${!empty generatedBeanClass}"><sm:bean className="ru.sibinco.scag.beans.${generatedBeanClass}"/></c:if></c:otherwise>
        </c:choose>
        <%--sm:message></sm:message--%>
<%
  //errorMessages=(java.util.List)request.getAttribute(ru.sibinco.scag.Constants.SCAG_ERROR_MESSAGES_ATTRIBUTE_NAME);
  if (errorMessages.size() > 0)
  {
%>
    <div class=content>
<%
   	for(java.util.Iterator it = errorMessages.iterator(); it.hasNext();)
   	{
   		ru.sibinco.scag.beans.MessageException exc = (ru.sibinco.scag.beans.MessageException) it.next();
      exc.printStackTrace(new java.io.PrintStream(System.err));
   		String code = exc.getCode();
      String text=exc.getMessage();
       //String text = exc.getCause() == null ? "" : "<br>Nested: \"" + exc.getCause().getMessage() + "\" (" + exc.getCause().getClass().getName() + ")";
           if(code.startsWith("errors."))
           {
%>
             <div class=error><div class=header>Error:</div><div class=header><%=text%></div></div>
<%
           }else if(code.startsWith("warning.")){
%>
            <div class=warning><div class=warning_header>Warning:</div><%=text%></div>
<%
           }else{
%>
            <div class=message><div class=message_header>Attention!</div><%=text%></div>
<%
           }
   	}
%>
    </div>
<%
   }
%>
        <c:choose>
          <c:when test="${!empty rawBody}">${rawBody}</c:when>
          <c:otherwise>
            <sm:content menu="${menu}" menu2="${menu2}">
              <jsp:doBody/>
            </sm:content>
<%--            <script>tableTag_checkChecks();</script>--%>
          </c:otherwise>
        </c:choose>
      </form>
    </td>
    <td width=30 background="content/images/smsc_07.jpg" rowspan=3></td>
    <td width=50 background="content/images/smsc_02.jpg" rowspan=3></td>
  </tr>
  <tr height="100%"><td style="position:relative;"><table cellspacing="0" cellpadding="0" height="100%" style="position: absolute; top:0; left:0; z-index: -1;">
      <tr>
        <td height="1px" width="12px" background="content/images/smsc_24.jpg"></td>
        <td height="1px"><img height=0px width="1px"></td>
        <td height="1px" width="12px" background="content/images/smsc_26.jpg"></td>
      </tr>
      </table>
    </td>
  </tr>
  <tr>
    <td>
      <table cellpadding="0" cellspacing="0">
        <tr>
          <td width="12" background="content/images/smsc_30.jpg"></td>
          <!--Find out the current year-->
          <%
          java.util.Calendar calendar = java.util.Calendar.getInstance(java.util.TimeZone.getDefault(), java.util.Locale.getDefault());
          calendar.setTime(new java.util.Date());
          int currentYear = calendar.get(java.util.Calendar.YEAR);
          %>  
          <td align="right" bgcolor="#161E49"><span style="color: #6F77A3;">Copyright&nbsp;&copy;&nbsp;EyeLine Communications, 2006&nbsp;-&nbsp;<%=currentYear%>&nbsp;</span></td>
          <td width="12" background="content/images/smsc_32.jpg"></td>
        </tr>
      </table>
    </td>
  </tr>
  </table>
</body>
<script>selectFirstTextInput();</script>
</html>
