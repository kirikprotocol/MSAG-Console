package mobi.eyeline.smsquiz.quizmanager.dirlistener;

import mobi.eyeline.smsquiz.quizmanager.QuizException;

import java.io.File;

/**
 * author: alkhal
 */
public class QuizFile {
    private String fileName;
    private long lastModified;

    public QuizFile(String fileName, long lastModified) throws QuizException {
        this.lastModified = lastModified;
        this.fileName = fileName;
    }

    public String getFileName() {
        return fileName;
    }

    public long getLastModified() {
        return lastModified;
    }
    public void modifyDate(long lastModified) {
        this.lastModified = lastModified;
    }
}
