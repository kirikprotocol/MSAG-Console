<%@
 taglib uri="http://java.sun.com/jsp/jstl/core" prefix="c"%><%@
 tag body-content="empty"                   %><%@
 attribute name="url"    required="true"    %><%@
 attribute name="name"    required="true"   %><%@
 attribute name="target"   required="false" %><%@
 attribute name="title"   required="true"   %><%@
 attribute name="onClick" required="false"
%><tr><td ID="${id}" title="${title}" nowrap
<%
  String servletPath = request.getServletPath();
  if (servletPath.startsWith("/"))
    servletPath = servletPath.substring(1);
  if (!url.endsWith(".jsp"))
  {
    final int pos = servletPath.lastIndexOf('/');
    if (pos >= 0)
      servletPath = servletPath.substring(0,pos);
  }
  if (url.equalsIgnoreCase(servletPath))
    out.print(" class='submenu_sel'");
%>>
<a href="${url}" target="${target}" onclick="${onclick}">${name}</a>
</td></tr>