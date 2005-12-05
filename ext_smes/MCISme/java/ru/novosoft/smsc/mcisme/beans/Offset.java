package ru.novosoft.smsc.mcisme.beans;

import ru.novosoft.smsc.util.config.Config;
import ru.novosoft.smsc.util.StringEncoderDecoder;

import javax.servlet.http.HttpServletRequest;
import java.util.List;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 20.07.2005
 * Time: 17:15:02
 * To change this template use File | Settings | File Templates.
 */
public class Offset extends MCISmeBean
{
    private String offsetName = "";
    private int offset = 0;
    private String regexp = "";

    private boolean initialized = false;
    private boolean createOffset  = false;
    private String  oldOffsetName = null;

    private String mbDone   = null;
    private String mbCancel = null;

    protected int init(List errors)
    {
        int result = super.init(errors);
        if (result != RESULT_OK) return result;

        if (!initialized && !createOffset)
        {
            if (offsetName == null || offsetName.length() == 0)
                return error("mcisme.error.tz_name");
            result = loadFromConfig(getConfig(), offsetName);
            if (result != RESULT_OK) return result;
            oldOffsetName = offsetName;
        }
        if (oldOffsetName == null) oldOffsetName = "";

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
        if (offsetName == null || offsetName.length() == 0)
            return error("mcisme.error.tz_name");

        Config config = getConfig();

        if (createOffset) // create new offset
        {
            if (containsInConfig(config, offsetName))
                return error("mcisme.error.tz_exists", offsetName);
        }
        else // edit old offset
        {
            if ( oldOffsetName != null && oldOffsetName.length() > 0) {
                if (!oldOffsetName.equals(offsetName) && containsInConfig(config, offsetName))
                    return error("mcisme.error.tz_exists", offsetName);
                removeFromConfig(config, oldOffsetName);
            }
        }

        storeToConfig(config, offsetName);
        getMCISmeContext().setChangedOffsets(true);
        return RESULT_DONE;
    }

    private boolean containsInConfig(Config config, String name) {
        return config.containsSection(OFFSETS_SECTION_NAME+'.'+StringEncoderDecoder.encodeDot(name));
    }
    private void removeFromConfig(Config config, String name) {
        config.removeSection(OFFSETS_SECTION_NAME+'.'+ StringEncoderDecoder.encodeDot(name));
    }
    private void storeToConfig(Config config, String name)
    {
        final String prefix = OFFSETS_SECTION_NAME+'.'+StringEncoderDecoder.encodeDot(name);
        config.setInt(prefix + ".offset", offset);
        config.setString(prefix + ".regexp", regexp);
    }
    private int loadFromConfig(Config config, String name)
    {
        final String prefix = OFFSETS_SECTION_NAME+'.'+StringEncoderDecoder.encodeDot(name);
        try
        {
            offset = config.getInt(prefix + ".offset");
            regexp = config.getString(prefix + ".regexp");
        }
        catch (Exception e) {
            return error("mcisme.error.config_load_failed", e);
        }
        return RESULT_OK;
    }

    public boolean isInitialized() {
        return initialized;
    }
    public void setInitialized(boolean initialized) {
        this.initialized = initialized;
    }

    public String getOldOffsetName() {
        return oldOffsetName;
    }
    public void setOldOffsetName(String oldOffsetName) {
        this.oldOffsetName = oldOffsetName;
    }

    public String getOffsetName() {
        return offsetName;
    }
    public void setOffsetName(String offsetName) {
        this.offsetName = offsetName;
    }

    public String getOffset() {
        return ((offset > 0) ? "+":"")+Integer.toString(offset);
    }
    public void setOffset(String offset) {
        try {
          if (offset.startsWith("+")) offset = offset.substring(1);
          this.offset = Integer.parseInt(offset);
        }
        catch(Exception e) {
          e.printStackTrace();
        }
    }

    public String getRegexp() {
        return regexp;
    }
    public void setRegexp(String regexp) {
        this.regexp = regexp;
    }

    public boolean isCreateOffset() {
        return createOffset;
    }
    public void setCreateOffset(boolean createOffset) {
        this.createOffset = createOffset;
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
