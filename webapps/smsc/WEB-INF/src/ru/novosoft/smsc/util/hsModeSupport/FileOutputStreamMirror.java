package ru.novosoft.smsc.util.hsModeSupport;

import ru.novosoft.smsc.admin.Constants;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;

public class FileOutputStreamMirror extends FileOutputStream {
    private FileOutputStream mirror = null;

    public FileOutputStreamMirror(String name) throws FileNotFoundException, IOException {
        this(name != null ? new File(name) : null, false);
    }

    public FileOutputStreamMirror(String name, boolean append) throws FileNotFoundException, IOException {
        this(name != null ? new File(name) : null, append);
    }

    public FileOutputStreamMirror(File file, boolean append) throws FileNotFoundException, IOException {
        super(file, append);
        if (Constants.isMirrorNeeded()) {
            File mirrorFile = Constants.getMirrorFile(file);
            mirror = new FileOutputStream(mirrorFile, append);
        }
    }

    public FileOutputStreamMirror(File file) throws FileNotFoundException, IOException {
        this(file, false);
    }

    public void close() throws IOException {
        super.close();
        if (mirror != null) mirror.close();
    }

    public void write(byte b[]) throws IOException {
        super.write(b);
        if (mirror != null) mirror.write(b);
    }

    public void write(byte b[], int off, int len) throws IOException {
        super.write(b, off, len);
        if (mirror != null) mirror.write(b, off, len);
    }

    public void write(int b) throws IOException {
        super.write(b);
        if (mirror != null) mirror.write(b);
    }
}
