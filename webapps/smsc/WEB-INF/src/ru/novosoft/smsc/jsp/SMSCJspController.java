package ru.novosoft.smsc.jsp;

import ru.novosoft.util.jsp.JspController;


public class SMSCJspController extends JspController
{
    SMSCAppContext appContext;

	protected ru.novosoft.util.jsp.AppContext initAppContext()
	{
		appContext = new SMSCAppContextImpl(getInitParameter("config"));
        return appContext;
	}

    public void destroy() {
        appContext.destroy();
        super.destroy();
    }
}

