package ru.novosoft.smsc.web;

import com.sun.facelets.FaceletViewHandler;
import org.apache.log4j.Logger;

import javax.faces.context.FacesContext;
import java.util.Locale;

/**
 * author: alkhal
 */
public class SmscViewHandler extends FaceletViewHandler{

  private static final Logger logger = Logger.getLogger(SmscViewHandler.class);

  public SmscViewHandler(javax.faces.application.ViewHandler parent) {
    super(parent);
  }

  @Override
  public Locale calculateLocale(FacesContext context) {
    System.out.println("Calculate locale");
    return super.calculateLocale(context);
  }
}
