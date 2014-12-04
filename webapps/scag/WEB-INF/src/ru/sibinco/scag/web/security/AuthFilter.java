package ru.sibinco.scag.web.security;

import javax.servlet.*;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import javax.servlet.http.HttpSession;
import java.io.IOException;
import java.util.HashSet;
import java.util.regex.Pattern;

public class AuthFilter implements Filter {

  protected static final org.apache.log4j.Category cat = org.apache.log4j.Category.getInstance(AuthFilter.class.getName());

  private Pattern passBy;
  public static final String USER_LOGIN_DATA = "userLoginData";
  public static final String PASSWORD = "password";
  public static final String USERNAME = "username";

  private String loginPage;
  private static String accessDeniedPage;
  private static String welcomePage;

  private String adminUserPassword;

  @Override
  public void init(FilterConfig config) throws ServletException {
    String s = config.getInitParameter("pass-by");
    if (s != null) {
      passBy = Pattern.compile(s);
      cat.info("uri's matches regular pattern " + s + " are unprotected");
    }

    loginPage = config.getInitParameter("login-page");
    if (loginPage == null) {
      cat.error("login-page is not defined in config");
      throw new ServletException("login-page is not defined in config");
    }

    accessDeniedPage = config.getInitParameter("access-denied-page");
    if (accessDeniedPage == null) {
      cat.error("access-denied-page is not defined in config");
      throw new ServletException("access-denied-page is not defined in config");
    }

    welcomePage = config.getInitParameter("welcome-page");
    if (welcomePage == null) {
      cat.error("welcome-page is not defined in config");
      throw new ServletException("welcome-page is not defined in config");
    }

    adminUserPassword = config.getInitParameter("admin.user.password");
  }

  @Override
  public void doFilter(ServletRequest request, ServletResponse response, FilterChain chain) throws IOException, ServletException {
    HttpServletRequest req = (HttpServletRequest) request;
    HttpServletResponse resp = (HttpServletResponse) response;
    HttpSession session = req.getSession();
    String uri = req.getRequestURI();
    String shortUri = uri.substring(req.getContextPath().length());

    if (passBy != null && passBy.matcher(shortUri).matches()) {
      //cat.debug("Resource with uri=" + shortUri + " allowed due to pass-by");
      chain.doFilter(request, response);
      return;
    }

    UserLoginData userLoginData = (UserLoginData) session.getAttribute(USER_LOGIN_DATA);
    if (userLoginData != null) {
      if(userLoginData.isURIPermitted(shortUri) || shortUri.equals(welcomePage) || shortUri.equals("/")){
        chain.doFilter(req, resp);
        return;
      } else {
        request.getRequestDispatcher(accessDeniedPage).forward(request, response);
        cat.warn("access to page uri=" + shortUri + " denied");
        return;
      }
    }

    String username = request.getParameter(USERNAME);
    String password = request.getParameter(PASSWORD);

    if (username == null || password == null) {
      request.setAttribute("uri", uri);
      request.getRequestDispatcher(loginPage).forward(request, response);
      cat.debug("Access to page uri='" + shortUri + "' has been denied.");
      return;
    }

    if (!username.equals("admin") || !password.equals(adminUserPassword)){
      request.setAttribute("uri", uri);
      request.setAttribute("showLanguageSelector", "true");
      request.getRequestDispatcher(loginPage).forward(request, response);
      cat.debug("access to page uri=" + shortUri + " denied");
      return;
    }

    userLoginData = new UserLoginData("admin", new HashSet<String>(){{add("super_admin");}}, new HashSet<String>());
    session.setAttribute(USER_LOGIN_DATA, userLoginData);

    if(userLoginData.isURIPermitted(shortUri) || shortUri.equals(welcomePage) || shortUri.equals("/")){
      chain.doFilter(req, resp);
    } else {
      request.getRequestDispatcher(accessDeniedPage).forward(request, response);
      cat.debug("Access to page uri='" + shortUri + "' has been denied");
    }
  }

  public void destroy() {

  }

}
