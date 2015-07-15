package ru.sibinco.scag.util;

import org.apache.log4j.Logger;
import org.apache.log4j.PropertyConfigurator;

import java.lang.reflect.Method;

import javax.servlet.jsp.*;
import javax.servlet.jsp.tagext.*;

public class LoggerTag extends BodyTagSupport {

  private Logger log = null;
  private String configFile = null;
  private String level = null;
  private static final String[] LEVELS =
      { "debug","info","warn","error","fatal"};

  public void setConfigFile(String fileName){

    this.configFile = fileName;

  }

  public void setLevel(String level){

    this.level = level;

  }

  public int doEndTag( ) throws JspException {

    String realPath = pageContext.getServletContext( ).getRealPath("/");
    String fileSep = System.getProperty("file.separator");

    if (realPath != null && (!realPath.endsWith(fileSep))){
      realPath = realPath + fileSep;}

    //configure the logger if the user provides this optional attribute
    if (configFile != null)
      PropertyConfigurator.configure(realPath +
          "WEB-INF/classes/" + configFile);

    //throw an exception if the tag user provides an invalid level,
    //something other than DEBUG, INFO, WARN, ERROR, or FATAL

    level = level.toLowerCase( );

    if (! contains(level))
      throw new JspException(
          "The value given for the level attribute is invalid.");

    //The logger has the same name as the class:
    //com.jspservletcookbook.LoggerTag. Therefore, it inherits its
    //appenders from a logger defined in the config file:
    //com.jspservletcookbook
    log = Logger.getLogger(LoggerTag.class);

    String message = getBodyContent( ).getString( ).trim( );
    Method method = null;

    try{

      method = log.getClass( ).
          getMethod(level,new Class[]{ Object.class });

      method.invoke(log,new String[]{message});

    } catch (Exception e){}

    return EVAL_PAGE;
  } // doEndTag

  public void release( ){

    //release resources used by instance variables
    log = null;
    configFile = null;
    level = null;

  }// release

  private boolean contains(String str){

    for (int i = 0; i < LEVELS.length; i++){

      if(LEVELS[i].equals(str))
        return true;
    }
    return false;
  }// contains
}
