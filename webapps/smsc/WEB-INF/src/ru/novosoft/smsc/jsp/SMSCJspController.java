package ru.novosoft.smsc.jsp;

import ru.novosoft.util.jsp.JspController;


public class SMSCJspController extends JspController
{
	protected ru.novosoft.util.jsp.AppContext initAppContext()
	{
		return new SMSCAppContextImpl(getInitParameter("config"));
	}
}

