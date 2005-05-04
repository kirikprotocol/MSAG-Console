package ru.novosoft.smsc.mcisme.beans;

import ru.novosoft.smsc.util.config.Config;
import ru.novosoft.smsc.util.StringEncoderDecoder;

import javax.servlet.http.HttpServletRequest;
import java.util.List;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 04.05.2005
 * Time: 16:25:37
 * To change this template use File | Settings | File Templates.
 */
public class Rule extends MCISmeBean
{
    private String ruleName = "";
    private int priority = 0;
    private String regexp = "";
    private int cause = 0;
    private boolean inform = false;

    private boolean initialized = false;
    private boolean createRule  = false;
    private String  oldRuleName = null;

    private String mbDone   = null;
    private String mbCancel = null;

    protected int init(List errors)
    {
        int result = super.init(errors);
        if (result != RESULT_OK) return result;

        if (!initialized && !createRule)
        {
            if (ruleName == null || ruleName.length() == 0)
                return error("Rule name not specified");
            result = loadFromConfig(getConfig(), ruleName);
            if (result != RESULT_OK) return result;
            oldRuleName = ruleName;
        }
        if (oldRuleName == null) oldRuleName = "";

        return result;
    }

    public int process(HttpServletRequest request)
    {
        int result = super.process(request);
        if (result != RESULT_OK) return result;

        if (mbCancel != null)    return RESULT_DONE;
        else if (mbDone != null) return done();

        return result;
    }

    private int done()
    {
        if (ruleName == null || ruleName.length() == 0)
            return error("Rule name not specified");

        Config config = getConfig();

        if (createRule) // create new rule
        {
            if (containsInConfig(config, ruleName))
                return error("Rule '"+ruleName+"' already exists");
        }
        else // edit old rule
        {
            if ( oldRuleName != null && oldRuleName.length() > 0) {
                if (!oldRuleName.equals(ruleName) && containsInConfig(config, ruleName))
                    return error("Rule '"+ruleName+"' already exists");
                removeFromConfig(config, oldRuleName);
            }
        }

        storeToConfig(config, ruleName);
        getMCISmeContext().setChangedRules(true);
        return RESULT_DONE;
    }

    private boolean containsInConfig(Config config, String name) {
        return config.containsSection(RULES_SECTION_NAME+'.'+StringEncoderDecoder.encodeDot(name));
    }
    private void removeFromConfig(Config config, String name) {
        config.removeSection(RULES_SECTION_NAME+'.'+ StringEncoderDecoder.encodeDot(name));
    }
    private void storeToConfig(Config config, String name)
    {
        final String prefix = RULES_SECTION_NAME+'.'+StringEncoderDecoder.encodeDot(name);
        config.setInt(prefix + ".priority", priority);
        config.setString(prefix + ".regexp", regexp);
        config.setInt(prefix + ".cause", cause);
        config.setInt(prefix + ".inform", inform ? 1:0);
    }
    private int loadFromConfig(Config config, String name)
    {
        final String prefix = RULES_SECTION_NAME+'.'+StringEncoderDecoder.encodeDot(name);
        try
        {
            priority = config.getInt(prefix + ".priority");
            regexp = config.getString(prefix + ".regexp");
            cause = config.getInt(prefix + ".cause");
            inform = (0 != config.getInt(prefix + ".inform"));
        }
        catch (Exception e) {
            return error("Load from config failed", e);
        }
        return RESULT_OK;
    }

    public boolean isInitialized() {
        return initialized;
    }
    public void setInitialized(boolean initialized) {
        this.initialized = initialized;
    }

    public String getOldRuleName() {
        return oldRuleName;
    }
    public void setOldRuleName(String oldRuleName) {
        this.oldRuleName = oldRuleName;
    }

    public String getRuleName() {
        return ruleName;
    }
    public void setRuleName(String ruleName) {
        this.ruleName = ruleName;
    }

    public int getPriority() {
        return priority;
    }
    public void setPriority(int priority) {
        this.priority = priority;
    }

    public String getRegexp() {
        return regexp;
    }
    public void setRegexp(String regexp) {
        this.regexp = regexp;
    }

    public int getCause() {
        return cause;
    }
    public void setCause(int cause) {
        this.cause = cause;
    }
    public boolean isInform() {
        return inform;
    }
    public void setInform(boolean inform) {
        this.inform = inform;
    }

    public boolean isCreateRule() {
        return createRule;
    }
    public void setCreateRule(boolean createRule) {
        this.createRule = createRule;
    }

    public String getMbDone() {
        return mbDone;
    }
    public void setMbDone(String mbDone) {
        this.mbDone = mbDone;
    }
    public String getMbCancel() {
        return mbCancel;
    }
    public void setMbCancel(String mbCancel) {
        this.mbCancel = mbCancel;
    }
}
