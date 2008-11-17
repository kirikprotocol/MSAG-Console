<%@ page import="ru.novosoft.smsc.infosme.beans.InfoSmeBean"%>
<%@ include file="/WEB-INF/inc/sme_menu.jsp"%>
<%
	  sme_menu_begin(out);
	  sme_menu_button(out, "mbMenu",  "quizes",      "Quizes",   "List of quizes");
    sme_menu_button(out, "mbMenu",  "reply_stats",      "Reply stats",      "Reply statistics");
    sme_menu_button(out, "mbMenu",  "quiz_result",   "Quiz results",  "Quiz results");
    sme_menu_button(out, "mbMenu",  "distr",   "Distribution",  "Distribution");
    sme_menu_space(out);
    sme_menu_button(out, "mbMenu",  "main",    "Main",    "Main page");
    sme_menu_button(out, "mbMenu",  "options",    "Options",    "Global SmsQuiz options");
    sme_menu_end(out);
%>