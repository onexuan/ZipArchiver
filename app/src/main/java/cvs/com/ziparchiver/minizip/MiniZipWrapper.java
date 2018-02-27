package cvs.com.ziparchiver.minizip;

import cvs.com.ziparchiver.R;

/**
 * @author igor on 07.11.17.
 */

public class MiniZipWrapper {

    static {
        System.loadLibrary("minizip");
    }

    private static final int[] ERROR_ID = {
            -3, -100, -101, -102, -103, -104
    };

    private static final int[] MESSAGE_ID = {
            R.string.error_wrong_password, R.string.error_create_zip,
            R.string.error_get_crc32, R.string.error_while_read,
            R.string.error_file_not_found, R.string.error_zip_file_not_found
    };

    public static int getErrorMessageById(int errorId) {
        for (int i = 0; i < ERROR_ID.length; i++) {
            if (errorId == ERROR_ID[i]) return MESSAGE_ID[i];
        }
        return R.string.error;
    }

    /**
     * @param zipfilename example /storage/emulated/0/Archive/img.zip
     * @param folderName  example List of '/storage/emulated/0/Archive/'
     * @param password    example 'test'
     */
    public native int createZipFromFolder(String zipfilename, String folderName, String password);

    /**
     * @param zipfilename example /storage/emulated/0/Archive/img.zip
     * @param filename    example /storage/emulated/0/Archive/img.jpg
     * @param password    example 'test'
     */
    public native int createZipFromFile(String zipfilename, String filename, String password);

    /**
     * @param zipfilename ex. /storage/emulated/0/Archive/img.zip
     * @param dirname     ex. /storage/emulated/0/Archive
     * @param password    ex. 'test'
     */
    public native int extractZip(String zipfilename, String dirname, String password);

    public native String getFilenameInZip(String zipfilename);

}
