<%@
 taglib uri="http://java.sun.com/jsp/jstl/core" prefix="c"%><%@
 tag body-content="empty"                  %><%@
 attribute name="changed" required="true"
%><c:choose><c:when test="${changed}"><span style="color:red">changed</span></c:when><c:otherwise><span style="color:green">ok</span></c:otherwise></c:choose>