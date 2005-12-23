<%@ page import="ru.novosoft.smsc.util.StringEncoderDecoder,
                 ru.novosoft.smsc.jsp.SMSCAppContext"%><!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<html><head>
<title><%=(BROWSER_TITLE==null) ? TITLE : BROWSER_TITLE%></title>
<base href="<%=request.getScheme()%>://<%=request.getServerName()%>:<%=request.getServerPort() + CPATH%>/">
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
<%if(browser_type == BROWSER_TYPE_MSIE) {
%><link rel="STYLESHEET" type="text/css" href="/styles/browser_ie.css"><%
} else if(browser_type == BROWSER_TYPE_MOZILLA) {
%><link rel="STYLESHEET" type="text/css" href="/styles/browser_mozilla.css"><%
} else if(browser_type == BROWSER_TYPE_SAFARI) {
%><link rel="STYLESHEET" type="text/css" href="/styles/browser_safari.css"><%
} else {
%><link rel="STYLESHEET" type="text/css" href="/styles/browser_unknown.css"><%
}%>
<script src="<%=getScriptsLocMsgFileName()%>" type="text/javascript"></script>
<script src="<%=getLocJSConfFileName()%>" type="text/javascript"></script>
<script src="/scripts/scripts.js" type="text/javascript"></script>
<%@include file="top_menu.jsp"%>
</head>
<%if(topMenu.size() > 0){%>
<body onload="initjsDOMenu()">
<%}else{%>
<body>
<%}%>
