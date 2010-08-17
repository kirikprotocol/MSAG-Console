package ru.novosoft.smsc.web.components;

import javax.faces.component.UIComponent;
import javax.faces.context.FacesContext;
import javax.faces.context.ResponseWriter;
import javax.faces.render.Renderer;
import java.io.IOException;
import java.io.Writer;

/**
 * @author Artem Snopkov
 */
public abstract class AjaxRenderer extends Renderer {

  protected abstract void encodeAjaxPart(javax.faces.context.FacesContext context, ResponseWriter w, javax.faces.component.UIComponent component) throws IOException;

  protected abstract void _encodeBegin(javax.faces.context.FacesContext context, javax.faces.component.UIComponent component) throws IOException;

  protected abstract void _encodeEnd(javax.faces.context.FacesContext context, javax.faces.component.UIComponent component) throws IOException;

  public final void encodeBegin(javax.faces.context.FacesContext context, javax.faces.component.UIComponent component) throws IOException {
    if ((context instanceof AjaxFacesContext))
      return;
    _encodeBegin(context, component);
  }

  public final void encodeEnd(FacesContext context, UIComponent component) throws IOException {
    if ((context instanceof AjaxFacesContext)) {
      AjaxFacesContext ctx = (AjaxFacesContext) context;
      if (ctx.getAjaxComponentId().equals(component.getId()))
        encodeAjaxPart(ctx, ctx.getAjaxResponseWriter(), component);
    } else
      _encodeEnd(context, component);
  }
}
