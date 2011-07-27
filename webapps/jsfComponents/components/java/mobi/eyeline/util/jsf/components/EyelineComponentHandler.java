package mobi.eyeline.util.jsf.components;

import com.sun.facelets.tag.MetaRuleset;
import com.sun.facelets.tag.jsf.ComponentConfig;
import com.sun.facelets.tag.jsf.ComponentHandler;

/**
 * User: artem
 * Date: 26.07.11
 */
public class EyelineComponentHandler extends ComponentHandler {
  public EyelineComponentHandler(ComponentConfig config) {
    super(config);
  }

  protected MetaRuleset createMetaRuleset(Class type) {
      return super.createMetaRuleset(type).alias("class", "styleClass");
  }
}
