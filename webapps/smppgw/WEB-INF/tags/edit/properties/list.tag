<%@taglib uri="http://java.sun.com/jsp/jstl/core" prefix="c"%><%@
 taglib uri="http://java.sun.com/jsp/jstl/functions" prefix="fn"%><%@
 taglib prefix="smf" uri="/smppgw/func"%><%@
 taglib prefix="sm-ep" tagdir="/WEB-INF/tags/edit/properties" %><%@
 tag body-content="empty" %><%@
 attribute name="title"       required="true"%><%@
 attribute name="name"        required="true"%><%@
 attribute name="values"      required="true"%><%@
 attribute name="valueTitles" required="true"%><%@
 attribute name="onChange"    required="false"%><%@
 attribute name="readonly"    required="false"%><%@
 attribute name="rowId" required="false"%>
<sm-ep:property title="${title}" rowId="${rowId}"
  ><c:set var="values" value="${fn:split(values, ',')}"
 /><c:set var="valueTitles" value="${fn:split(valueTitles, ',')}"
 /><c:choose><c:when test="${readonly}"><input class=txt type="text" readonly name="${name}" value="${bean[name]}"></c:when><c:otherwise><div class=select><select name="${name}" <c:if test="${!empty onChange}">onChange="${onChange}"</c:if> >
<c:forEach var="i" items="${values}" varStatus="st">
  <option value="${i}" <c:if test="${i == bean[name]}">selected</c:if>>${valueTitles[st.count-1]}</option>
</c:forEach>
</select></div></c:otherwise></c:choose></sm-ep:property>