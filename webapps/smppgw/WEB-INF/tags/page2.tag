<%@
 taglib prefix="sm" tagdir="/WEB-INF/tags"%><%@
 tag body-content="empty"              %><%@
 attribute name="beanClass" required="false" %><%@
 attribute name="title"     required="true" %><%@
 attribute name="menu1"      required="false" %><%@
 attribute name="menu2"      required="false" %><%@
 attribute name="menu3"      required="false" %><%@
 attribute name="form_method"     required="false" %><%@
 attribute name="form_uri"     required="false" %><%@
 attribute name="form_enctype"     required="false"%><%@
 attribute name="onLoad"     required="false"%><%@
 attribute name="body1"     required="true"%><%@
 attribute name="body2"     required="false"%><sm:page
  beanClass="${beanClass}"
  title="${title}"
  form_method="${form_method}"
  form_uri="${form_uri}"
  form_enctype="${form_enctype}"
  onLoad="${onLoad}"><jsp:attribute name="rawBody">${menu1}${body1}${menu2}${body2}${menu3}</jsp:attribute></sm:page>