package cvs.com.ziparchiver;

import android.Manifest;
import android.annotation.SuppressLint;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.annotation.RequiresPermission;
import android.support.v4.app.ActivityCompat;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.widget.Toast;

import java.io.File;

import cvs.com.ziparchiver.minizip.MiniZipWrapper;

public class MainActivity extends AppCompatActivity {

    private static final String TAG = MainActivity.class.getSimpleName();
    private static final int REQUEST_PERMISSION_CODE = 101;
    private static final String PASSWORD = "1";

    private final FolderManager folderManager = new FolderManager();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
    }

    @Override
    protected void onStart() {
        super.onStart();
        ActivityCompat.requestPermissions(this,
                new String[]{Manifest.permission.WRITE_EXTERNAL_STORAGE},
                REQUEST_PERMISSION_CODE
        );
    }

    @SuppressLint("MissingPermission")
    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        switch (requestCode) {
            case REQUEST_PERMISSION_CODE:
                if (grantResults.length > 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                    testZip();
                } else {
                    Toast.makeText(this, "Require permission", Toast.LENGTH_SHORT).show();
                }
                break;
        }
    }

    @RequiresPermission(Manifest.permission.WRITE_EXTERNAL_STORAGE)
    private void testZip() {
        encryptFolder();
        decryptFolder();
    }

    private void decryptFolder() {
        final File archiveFolder = folderManager.getArchiveFolder();
        final String zipPath = folderManager.getArchiveFile().getAbsolutePath();
        final MiniZipWrapper wrapper = new MiniZipWrapper();
        final int result = wrapper.extractZip(zipPath, archiveFolder.getAbsolutePath() + "Unzip", PASSWORD);
        Log.d(TAG, "createZip: " + result);
        Toast.makeText(this, "Folder decrypted, result:" + result, Toast.LENGTH_SHORT).show();
    }

    private void encryptFolder() {
        final File archiveFolder = folderManager.getArchiveFolder();
        folderManager.stubIfEmpty(archiveFolder);
        final String zipPath = folderManager.getArchiveFile().getAbsolutePath();
        final MiniZipWrapper wrapper = new MiniZipWrapper();
        int result = wrapper.createZipFromFolder(zipPath, archiveFolder.getAbsolutePath(), PASSWORD);
        Log.d(TAG, "createZip: " + result);
        Toast.makeText(this, "Folder encrypted, result:" + result, Toast.LENGTH_SHORT).show();
    }

}
