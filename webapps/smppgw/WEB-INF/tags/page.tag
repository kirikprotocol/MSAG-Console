<%@
 taglib uri="http://java.sun.com/jsp/jstl/core" prefix="c"%><%@
 taglib prefix="sm" tagdir="/WEB-INF/tags"%><%@
 taglib prefix="sm-mm" tagdir="/WEB-INF/tags/main_menu"%><%@

 tag body-content="scriptless"              %><%@
 attribute name="beanClass" required="false" %><%@
 attribute name="title"     required="true" %><%@
 attribute name="menu"      required="false" %><%@
 attribute name="form_method"     required="false" %><%@
 attribute name="form_uri"     required="false" %><%@
 attribute name="form_enctype"     required="false"
%><%
  if (pageContext.getAttribute("beanClass") == null)
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
      Thread.currentThread().getContextClassLoader().loadClass("ru.sibinco.smppgw.beans." + generatedBeanClass);
      pageContext.setAttribute("generatedBeanClass", generatedBeanClass);
    } catch(ClassNotFoundException e) {
      //skip it
    }
  }
%><!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<html>
<head>
  <title>SMPP Gateway Administration Console<c:if test="${!empty title}">: ${title}</c:if></title>
  <base href="<%=request.getScheme()%>://<%=request.getServerName()%>:<%=request.getServerPort()+request.getContextPath()%>/">
  <link rel="STYLESHEET" type="text/css" href="/styles/colors.css">
  <link rel="STYLESHEET" type="text/css" href="/styles/common.css">
  <link rel="STYLESHEET" type="text/css" href="/styles/list.css">
  <link rel="STYLESHEET" type="text/css" href="/styles/properties_list.css">
  <link rel="STYLESHEET" type="text/css" href="/styles/main_menu.css">
  <link rel="STYLESHEET" type="text/css" href="/styles/collapsing_tree.css">
  <link rel="STYLESHEET" type="text/css" href="/styles/collapsing_list.css">
  <link rel="STYLESHEET" type="text/css" href="/styles/navbar.css">
  <link rel="STYLESHEET" type="text/css" href="/styles/messages.css">
  <link rel="STYLESHEET" type="text/css" href="/styles/page_menu.css">
  <link rel="STYLESHEET" type="text/css" href="/styles/sme_menu.css">

  <link rel="STYLESHEET" type="text/css" href="collapsing_tree.css">

  <script src="/scripts/scripts.js" type="text/javascript"></script>
</head>
<body>
  <script>
  function openPerfMon()
  {
    open("perfmon/index.jsp", null, "channelmode=no,directories=no,fullscreen=no,location=no,menubar=no,resizable=yes,scrollbars=no,status=no,titlebar=no,toolbar=no,height=450,width=580");
    return false;
  }
  </script>

  <table height="100%" cellspacing=0 cellpadding=0 class=main_table>
  <tr>
    <td width=50 background="/images/smsc_02.jpg" rowspan=3></td>
    <td width=30 background="/images/smsc_03.jpg" rowspan=3></td>
    <td bgcolor="#F3F0E3" style="position: relative; top:0; left:0;">
      <!-- background -->
      <table cellspacing=0 cellpadding=0 height="100%" style="position: absolute; top:0; left:0; z-index: -1;">
      <tr>
        <td width=12px background="/images/smsc_24.jpg"></td>
        <td>&nbsp;</td>
        <td width=12px background="/images/smsc_26.jpg"></td>
      </tr>
      </table>
      <!-- "SMS Service Center" header -->
      <table cellspacing=0 cellpadding=0>
      <tr>
        <td width=236><IMG SRC="/images/smsc_04.jpg" WIDTH=236 HEIGHT=45 ALT=""></td>
        <td bgcolor="#161E49"><span style="color: #6F77A3; font: bold 11px; vertical-align: -10px;">Version&nbsp;1.0</span></td>
        <td width=12 background="/images/smsc_06.jpg"></td>
      </tr>
      </table>
      <!-- main menu -->
      <sm-mm:menu>
        <sm-mm:section menuId="MENU_SMPPGW_menu" submenuId="MENU_SMPPGW_submenu" name="SMPP&nbsp;Gateway">
          <sm-mm:item url="gw/config"                name="Configuration"   title=""/>
          <sm-mm:item url="gw/providers"             name="Providers"       title=""/>
          <sm-mm:item url="gw/smscs"                 name="Service centers" title=""/>
          <sm-mm:item url="smsc_service/status.jsp"  name="Status"          title=""/>
          <sm-mm:item url="smsc_service/logging.jsp" name="Logging"         title=""/>
          <sm-mm:item url="gw/users"                 name="Users"           title=""/>
          <sm-mm:item url="gw/resources"             name="Resources"       title=""/>
          <sm-mm:item url="journal"                  name="Journal"         title=""/>
        </sm-mm:section>

        <sm-mm:section menuId="MENU0_Statistics_menu" submenuId="MENU0_Statistics_submenu" name="Statistics">
          <sm-mm:item url="perfmon"  name="Monitor"    target="SMSC Perfomance monitor" title="SMSC Perfomance monitor" onClick="return openPerfMon();"/>
          <sm-mm:item url="smsstat"  name="Statistics" title=""/>
        </sm-mm:section>

        <sm-mm:section menuId="MENU0_Routing_menu" submenuId="MENU0_Routing_submenu" name="Routing">
          <sm-mm:item url="routing/subjects" name="Subjects"       title=""/>
          <sm-mm:item url="routing/routes"   name="Routes"         title=""/>
          <sm-mm:item url="routing/tracer"   name="Routes tracing" title=""/>
        </sm-mm:section>

        <sm-mm:section menuId="MENU0_Services_menu" submenuId="MENU0_Services_submenu" name="Services">
          <sm-mm:item url="services/sme" name="SME's" title=""/>
        </sm-mm:section>
      </sm-mm:menu>

      <table id=MAIN_MENU_BACKGROUND_TABLE border=0 cellspacing=0 cellpadding=0 class=main_menu_background>
        <tr>
          <td width=80 background="/images/smsc_13.jpg"></td>
          <td background="/images/smsc_14.jpg">&nbsp;</td>
          <td width=80 background="/images/smsc_16.jpg"></td>
        </tr>
      </table>
      <span id="MENU0_NONE"></span>
      <table cellpadding=0 cellspacing=0 height=30px class=smsc_status>
        <tr>
          <th background="/images/smsc_17.jpg" nowrap>${title}</th>
          <td >&nbsp;</td>
          <td width=12px background="/images/smsc_19.jpg" style="padding-right:0px;"></td>
        </tr>
      </table>

      <form name="opForm" id=opForm
                <c:if test="${!empty form_uri}">action="${form_uri}"</c:if>
                method="<c:choose><c:when test="${!empty form_method}">${form_method}</c:when><c:otherwise>post</c:otherwise></c:choose>"
                <c:if test="${!empty form_method}">method="${form_method}"</c:if>
                <c:if test="${!empty form_enctype}">enctype="${form_enctype}"</c:if>
                onSubmit="return validateForm(this)">
        <input type=hidden ID=jbutton value=jbutton>

        <c:choose>
          <c:when test="${!empty beanClass}"><sm:bean className="ru.sibinco.smppgw.beans.${beanClass}"/></c:when>
          <c:otherwise><c:if test="${!empty generatedBeanClass}"><sm:bean className="ru.sibinco.smppgw.beans.${generatedBeanClass}"/></c:if></c:otherwise>
        </c:choose>
        <sm:content menu="${menu}">
          <jsp:doBody/>
        </sm:content>

      </form>
    </td>
    <td width=30 background="/images/smsc_07.jpg" rowspan=3></td>
    <td width=50 background="/images/smsc_02.jpg" rowspan=3></td>
  </tr>
  <tr height="100%"><td style="position:relative;"><table cellspacing="0" cellpadding="0" height="100%" style="position: absolute; top:0; left:0; z-index: -1;">
      <tr>
        <td height="1px" width="12px" background="/images/smsc_24.jpg"></td>
        <td height="1px"><img height=0px width="1px"></td>
        <td height="1px" width="12px" background="/images/smsc_26.jpg"></td>
      </tr>
      </table>
    </td>
  </tr>
  <tr>
    <td>
      <table cellpadding="0" cellspacing="0">
        <tr>
          <td width="12" background="/images/smsc_30.jpg"></td>
          <td align="right" bgcolor="#161E49"><span style="color: #6F77A3;">Copyright&nbsp;&copy;&nbsp;2003.&nbsp;All&nbsp;rights&nbsp;reserved</span></td>
          <td width="12" background="/images/smsc_32.jpg"></td>
        </tr>
      </table>
    </td>
  </tr>
  </table>
</body>
<script>selectFirstTextInput();</script>
</html>
