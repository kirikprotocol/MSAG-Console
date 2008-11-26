<%@ page import="ru.novosoft.smsc.infosme.beans.InfoSmeBean"%>
<%@ include file="/WEB-INF/inc/sme_menu.jsp"%>
<%
	  sme_menu_begin(out);
	  sme_menu_button(out, "mbMenu",  "quizes",      getLocString("smsquiz.label.quizes"),   getLocString("smsquiz.label.quizes.info"));
    sme_menu_button(out, "mbMenu",  "distr",   getLocString("smsquiz.label.messages"),  getLocString("smsquiz.label.messages.info"));
    sme_menu_button(out, "mbMenu",  "reply_stats",      getLocString("smsquiz.label.replies"),      getLocString("smsquiz.label.replies.info"));
    sme_menu_button(out, "mbMenu",  "quiz_result",   getLocString("smsquiz.label.results"),  getLocString("smsquiz.label.results.info"));
    sme_menu_space(out);
    sme_menu_button(out, "mbMenu",  "main",    getLocString("smsquiz.label.main"),    getLocString("smsquiz.label.main.info"));
    sme_menu_button(out, "mbMenu",  "options",   getLocString( "smsquiz.label.options"),    getLocString("smsquiz.label.options.info"));
    sme_menu_end(out);
%>