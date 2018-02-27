package cvs.com.ziparchiver;

import android.os.Environment;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.PrintWriter;
import java.io.UnsupportedEncodingException;
import java.util.Random;

/**
 * @author igor on 07.11.17.
 */

public class FolderManager {

    private static final String TAG = FolderManager.class.getSimpleName();
    private static final String ARCHIVE_NAME = "Archive";
    private static final String MAIN_FOLDER = Environment.getExternalStorageDirectory().getAbsolutePath() +
            File.separator;
    private static final String ARCHIVE_FOLDER = MAIN_FOLDER + ARCHIVE_NAME;
    private static final String ARCHIVE_EXTENSION = ".zip";
    private static final String ARCHIVE_FILE = ARCHIVE_FOLDER + ARCHIVE_EXTENSION;

    public String getArchiveName() {
        return ARCHIVE_NAME + ARCHIVE_EXTENSION;
    }

    public File getArchiveFile() {
        return new File(ARCHIVE_FILE);
    }

    public File getArchiveFolder() {
        return new File(ARCHIVE_FOLDER);
    }

    public void stubIfEmpty(File folder) {
        if (!folder.exists()) {
            folder.mkdirs();
        }
        final File stubFile = new File(folder, "test.txt");
        try {
            createRandomFileContent(stubFile);
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        } catch (UnsupportedEncodingException e) {
            e.printStackTrace();
        }
    }

    private void createRandomFileContent(File stubFile) throws FileNotFoundException, UnsupportedEncodingException {
        long count = 0;
        PrintWriter writer = new PrintWriter(stubFile, "UTF-8");
        Random random = new Random();
        for (int i = 0; i < 4096; i++) {
            char[] word = new char[random.nextInt(8) + 3]; // words of length 3 through 10. (1 and 2 letter words are boring.)
            count += word.length;
            for (int j = 0; j < word.length; j++) {
                word[j] = (char) ('a' + random.nextInt(26));

            }
            writer.print(new String(word) + ' ');
            count += 1;
            if (i % 10 == 0) {
                writer.println();
                count += 2;
            }
        }
        writer.close();
    }

    public File getRootFolder() {
        final File file = new File(MAIN_FOLDER);
        file.mkdirs();
        return file;
    }

}