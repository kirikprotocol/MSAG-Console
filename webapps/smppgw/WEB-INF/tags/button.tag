<%@
 taglib uri="http://java.sun.com/jsp/jstl/core" prefix="c"%><%@
 tag body-content="empty"                  %><%@
 attribute name="type"    required="false" %><%@
 attribute name="img"     required="false" %><%@
 attribute name="name"    required="false"  %><%@
 attribute name="value"   required="false"  %><%@
 attribute name="title"   required="true"  %><%@
 attribute name="onClick" required="false" %><%@
 attribute name="isCheckSensible" required="false"
%><img src="/images/<c:choose><c:when test="${type == 'add'}">but_add.gif</c:when><c:when test="${type == 'del'}">but_del.gif</c:when><c:otherwise>${img}</c:otherwise></c:choose>" class=button jbuttonName="${name}" jbuttonValue="${value}" title="${title}" jbuttonOnclick="${onClick}" <c:if test="${isCheckSensible}">isCheckSensible="${isCheckSensible}"</c:if>>