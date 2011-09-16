package ru.novosoft.smsc.web;

import mobi.eyeline.util.jsf.components.TrinidadAlternateViewHandler;

import javax.faces.application.ViewHandler;
import javax.faces.context.FacesContext;
import java.util.Locale;

/**
 * author: alkhal
 */
public class SmscViewHandler extends TrinidadAlternateViewHandler {

  public SmscViewHandler(ViewHandler parent) {
    super(parent);
  }

  @Override
  public Locale calculateLocale(FacesContext context) {
    Locale l = (Locale)context.getExternalContext().getRequestMap().get(LocaleFilter.LOCALE_PARAMETER);
    if(l != null) {
      return l;
    }else {
      return super.calculateLocale(context);
    }
  }
}
