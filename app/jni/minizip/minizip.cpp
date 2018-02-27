#include <jni.h>
#include "cvs_com_ziparchiver_minizip_MiniZipWrapper.h"
#include "unzip.h"
#include "zip.h"

#include <string.h>
#include <time.h>
#include <unistd.h>
#include <utime.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <dirent.h>
#include <limits.h> /* PATH_MAX */

#include <android/log.h>

#define MKDIR(d) mkdir(d, 0775)

const int WRITE_BUFFER_SIZE = 16384;
const int MAX_FILENAME_LEN = 256;

// Errors id
const int ERROR_CREATE_ZIP = -100;
const int ERROR_GET_CRC32 = -101;
const int ERROR_WHILE_READ = -102;
const int ERROR_FILE_NOT_FOUND = -103;
const int ERROR_ZIP_FILE_NOT_FOUND = -104;
const int ERROR_ZIP_FILE = -105;

static const char *curDir = ".";
static const char *parDir = "..";

#if defined(WIN32) || defined(_WIN32)
#define PATH_SEPARATOR "\\"
#else
#define PATH_SEPARATOR "/"
#endif

void getFileTime(const char *filename, tm_zip *tmzip, uLong *dostime) {
    struct stat s = { 0 };
    time_t tm_t = 0;

    if (strcmp(filename, "-") != 0) {
        char name[MAX_FILENAME_LEN + 1];

        int len = strlen(filename);
        if (len > MAX_FILENAME_LEN) {
            len = MAX_FILENAME_LEN;
        }

        strncpy(name, filename, MAX_FILENAME_LEN - 1);
        name[MAX_FILENAME_LEN] = 0;

        if (name[len - 1] == '/') {
            name[len - 1] = 0;
        }

        /* not all systems allow stat'ing a file with / appended */
        if (stat(name, &s) == 0) {
            tm_t = s.st_mtime;
        }
    }

    struct tm* filedate = localtime(&tm_t);
    tmzip->tm_sec  = filedate->tm_sec;
    tmzip->tm_min  = filedate->tm_min;
    tmzip->tm_hour = filedate->tm_hour;
    tmzip->tm_mday = filedate->tm_mday;
    tmzip->tm_mon  = filedate->tm_mon;
    tmzip->tm_year = filedate->tm_year;
}

void setFileTime(const char *filename, uLong dosdate, tm_unz tmu_date) {
    struct tm newdate;
    newdate.tm_sec  = tmu_date.tm_sec;
    newdate.tm_min  = tmu_date.tm_min;
    newdate.tm_hour = tmu_date.tm_hour;
    newdate.tm_mday = tmu_date.tm_mday;
    newdate.tm_mon  = tmu_date.tm_mon;

    if (tmu_date.tm_year > 1900) {
        newdate.tm_year = tmu_date.tm_year - 1900;
    } else {
        newdate.tm_year = tmu_date.tm_year;
    }
    newdate.tm_isdst = -1;

    struct utimbuf ut;
    ut.actime = ut.modtime = mktime(&newdate);
    utime(filename, &ut);
}

int isLargeFile(const char* filename) {
    FILE* pFile = fopen64(filename, "rb");
    if (pFile == NULL) return 0;

    fseeko64(pFile, 0, SEEK_END);
    ZPOS64_T pos = ftello64(pFile);
    fclose(pFile);

    return (pos >= 0xffffffff);
}

// Calculate the CRC32 of a file
int getCRC32(const char* filenameinzip, Bytef *buf, unsigned long size_buf, unsigned long* result_crc) {
    unsigned long calculate_crc = 0;

    int status = ZIP_OK;

    FILE *fin = fopen64(filenameinzip, "rb");
    if (fin == NULL){
        status = ERROR_GET_CRC32;
    }
    else {
        unsigned long size_read = 0;
        do {
            size_read = (int) fread(buf, 1, size_buf, fin);

            if ((size_read < size_buf) && (feof(fin) == 0)) {
                status = ERROR_WHILE_READ;
            }

            if (size_read > 0) {
                calculate_crc = crc32(calculate_crc, buf, size_read);
            }
        } while ((status == ZIP_OK) && (size_read > 0));
    }

    if (fin) {
        fclose(fin);
    }

    *result_crc = calculate_crc;
    return status;
}

int extractCurrentFile(unzFile uf, const char * dirname, const char *password) {
    unz_file_info64 file_info = { 0 };
    char filename_inzip[MAX_FILENAME_LEN] = { 0 };

    int status = unzGetCurrentFileInfo64(uf, &file_info, filename_inzip, sizeof(filename_inzip), NULL, 0, NULL, 0);
    if (status != UNZ_OK) {
    	return status;
    }

    uInt size_buf = WRITE_BUFFER_SIZE;
    void* buf = (void*) malloc(size_buf);
    if (buf == NULL) return UNZ_INTERNALERROR;

    status = unzOpenCurrentFilePassword(uf, password);
    char* write_filename = filename_inzip;
    chdir(dirname);

     __android_log_print(ANDROID_LOG_ERROR, "Kurwa", "--------------START-------------");
    __android_log_print(ANDROID_LOG_ERROR, "Kurwa", "Relative file name %s", write_filename);
    __android_log_print(ANDROID_LOG_ERROR, "Kurwa", "Start dirname name %s", dirname);

    if(strncmp(PATH_SEPARATOR, write_filename, 1) == 0) {
        char * tmpdirname = strdup(dirname);
        char * basePath = (char *) malloc(sizeof(char) * (strlen(tmpdirname) + 1)) ;
        strcpy(basePath, tmpdirname);
        strcat(basePath, PATH_SEPARATOR);
        char *token, *str, *tofree;
        tofree = str = strdup(write_filename);  // We own str's memory now.
        token = strsep(&str, PATH_SEPARATOR);
        do {
        __android_log_print(ANDROID_LOG_ERROR, "Kurwa", "each token %s", token);
              if(token && (strlen(token) > 0)) {
                  __android_log_print(ANDROID_LOG_ERROR, "Kurwa", "old len %d", strlen(basePath));
                  int oldLen = strlen(basePath);
                  int newLen = oldLen + strlen(token) + 1;
                  __android_log_print(ANDROID_LOG_ERROR, "Kurwa", "new len %d", newLen);
                  basePath = (char *) realloc(basePath, (sizeof(char) * newLen));
                  __android_log_print(ANDROID_LOG_ERROR, "Kurwa", "path after relocate %s", basePath);
                  basePath[oldLen] = '\0';
                  __android_log_print(ANDROID_LOG_ERROR, "Kurwa", "path after terminator relocate %s", basePath);
                  __android_log_print(ANDROID_LOG_ERROR, "Kurwa", "path after relocate oldLen  %c", basePath[oldLen - 1]);
                  __android_log_print(ANDROID_LOG_ERROR, "Kurwa", "path after relocate oldLen  %c", basePath[oldLen - 2]);
                  __android_log_print(ANDROID_LOG_ERROR, "Kurwa", "path after relocate oldLen  %c", basePath[oldLen - 3]);
                  __android_log_print(ANDROID_LOG_ERROR, "Kurwa", "path after relocate newLen  %c", basePath[newLen - 1]);
                  strcat(basePath, token);
                  strcat(basePath, PATH_SEPARATOR);
                  strcpy(write_filename, token);
                  __android_log_print(ANDROID_LOG_ERROR, "Kurwa", "path after concat %s", basePath);
                   basePath[newLen] = '\0';
                  __android_log_print(ANDROID_LOG_ERROR, "Kurwa", "path after terminator concat %s", basePath);
              }
              token = strsep(&str, PATH_SEPARATOR);
              if(token && (strlen(token) > 0)) {
                  int mkdirResult = MKDIR(basePath);
                                    __android_log_print(ANDROID_LOG_ERROR, "Kurwa", "MKDir %s", basePath);

                  chdir(basePath);
              }
        } while (token);
        free(tofree);
        free(basePath);
    }

    // Create the file on disk so we can unzip to it
    FILE* fout = NULL;
    if (status == UNZ_OK) {
          fout = fopen64(write_filename, "wb");
    }

    __android_log_print(ANDROID_LOG_ERROR, "Kurwa", "File name at the end %s", write_filename);
    __android_log_print(ANDROID_LOG_ERROR, "Kurwa", "Folder dirname at end %s", dirname);


    // Read from the zip, unzip to buffer, and write to disk
    if (fout != NULL) {
        do {
            status = unzReadCurrentFile(uf, buf, size_buf);
            if (status <= 0) break;
            if (fwrite(buf, status, 1, fout) != 1) {
                status = UNZ_ERRNO;
                break;
            }
        } while (status > 0);

        if (fout) fclose(fout);

        // Set the time of the file that has been unzipped
        if (status == 0) {
        	setFileTime(write_filename, file_info.dosDate, file_info.tmu_date);
        }
    }

    unzCloseCurrentFile(uf);

    free(buf);

    __android_log_print(ANDROID_LOG_ERROR, "Kurwa", "--------------END-------------");

    return status;
}

/////////////////////////////////////////////////////////

void removeSubstring(char *s,const char *toremove) {
  while((s = strstr(s,toremove))) {
    memmove(s,s+strlen(toremove), 1+strlen(s+strlen(toremove)));
    }
}

char * getRealmPath(const char * folderName, const char * fileName) {
    char buf[PATH_MAX];

    int newLenght = strlen(folderName) + 1 + strlen(fileName);
    char newPath[newLenght + 1];

    strcpy(newPath, folderName);
    strcat(newPath, PATH_SEPARATOR);
    strcat(newPath, fileName);
    realpath(newPath, buf);
    return strdup(buf);
}

void iterateFolder(const char * rootpath,const char * filename, Bytef* buf, const zipFile zf, int size_buf,
    const char * password, int opt_compress_level) {
    DIR *dir = dir = opendir(filename);
    struct dirent *drnt;
    char *name;
    while ((drnt = readdir(dir)) != NULL) {
         name = drnt->d_name;
        unsigned char type = drnt->d_type;
        if ((strcmp (name, curDir) != 0) && (strcmp (name, parDir) != 0)) {
            if (type == DT_DIR) {
                char * newPath = getRealmPath(filename, name);
                iterateFolder(rootpath, newPath, buf, zf, size_buf, password, opt_compress_level);
            } else {
                const char * fullName = getRealmPath(filename, name);
                // Get information about the file on disk so we can store it in zip
                zip_fileinfo zi = { 0 };
                getFileTime(fullName, &zi.tmz_date, &zi.dosDate);
                int status = 0;
                unsigned long crcFile = 0;
                status = getCRC32(fullName, buf, size_buf, &crcFile);

                int zip64 = isLargeFile(fullName);

                // Construct the filename that our file will be stored in the zip as.
                const char *savefilenameinzip = fullName;
                {
                    const char *tmpptr = NULL;
                    const char *lastslash = 0;
                    for (tmpptr = savefilenameinzip; *tmpptr; tmpptr++) {
                        if (*tmpptr == '\\' || *tmpptr == '/') {
                            lastslash = tmpptr;
                        }
                    }
                    if (lastslash != NULL) {
                        savefilenameinzip = lastslash + 1;
                    }
                }

                  char * relativePath = (char *) calloc(1, strlen(filename) + strlen(PATH_SEPARATOR) +
                                               strlen(savefilenameinzip) + 1);

                 strcpy(relativePath, filename);
                 removeSubstring(relativePath, rootpath);
                 if(relativePath &&  strlen(relativePath) > 0) {
                    strcat(relativePath, PATH_SEPARATOR);
                 }
                 strcat(relativePath, savefilenameinzip);
                // Create zip file
                status = zipOpenNewFileInZip3_64(zf, relativePath, &zi, NULL, 0, NULL, 0, NULL /* comment*/,
                        (opt_compress_level != 0) ? Z_DEFLATED : 0, opt_compress_level, 0,
                        -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY, password, crcFile, zip64);
                if(status == ZIP_OK) {
                     FILE *fin = NULL;
                     fin = fopen64(fullName, "rb");
                     if (fin == NULL) {
                             status = 1;
                     }
                     if(status == 0) {
                         int size_read = 0;
                         do {
                             size_read = (int) fread(buf, 1, size_buf, fin);
                             if ((size_read < size_buf) && (feof(fin) == 0)) {
                                 status = 1;
                             }

                             if (size_read > 0) {
                                 status = zipWriteInFileInZip(zf, buf, size_read);
                             }
                         } while ((status == 0) && (size_read > 0));
                     }
                     if (fin) {
                         fclose(fin);
                     }
                     if(zf) {
                        zipCloseFileInZip(zf);
                     }
                 }
                 free(relativePath);
            }
        }
    }
    if(dir) {
        closedir(dir);
    }
}

JNIEXPORT jint JNICALL Java_cvs_com_ziparchiver_minizip_MiniZipWrapper_createZipFromFolder(
  JNIEnv * env, jobject, jstring zipfileStr, jstring foldernameStr, jstring passwordStr) {

    jboolean isCopy;
    const char * zipfilename = env->GetStringUTFChars(zipfileStr, &isCopy);
    const char * filename = env->GetStringUTFChars(foldernameStr, &isCopy);
    const char * password = env->GetStringUTFChars(passwordStr, &isCopy);

    int status = 0;
    int opt_compress_level = Z_DEFAULT_COMPRESSION;

    // Create archive zipfilename
    zipFile zf = zipOpen64(zipfilename, APPEND_STATUS_CREATE);
    if (zf == NULL) {
        status = ERROR_CREATE_ZIP;
    }

    int size_buf = WRITE_BUFFER_SIZE;
    Bytef* buf = (Bytef*) malloc(size_buf);

    iterateFolder(filename, filename, buf, zf, size_buf,password, opt_compress_level);

    zipClose(zf, NULL);

    // Release memory
    free(buf);
    env->ReleaseStringUTFChars(zipfileStr, zipfilename);
    env->ReleaseStringUTFChars(foldernameStr, filename);
    env->ReleaseStringUTFChars(passwordStr, password);

    return status;
}

//////////////////////////////////////////////////////////

JNIEXPORT jint JNICALL Java_cvs_com_ziparchiver_minizip_MiniZipWrapper_createZipFromFile(
  JNIEnv * env, jobject, jstring zipfileStr, jstring filenameStr, jstring passwordStr) {

    jboolean isCopy;
    const char * zipfilename = env->GetStringUTFChars(zipfileStr, &isCopy);
    const char * filename = env->GetStringUTFChars(filenameStr, &isCopy);
    const char * password = env->GetStringUTFChars(passwordStr, &isCopy);

    int status = 0;
    int opt_compress_level = Z_DEFAULT_COMPRESSION;

    // Create archive zipfilename
    zipFile zf = zipOpen64(zipfilename, APPEND_STATUS_CREATE);
    if (zf == NULL) {
        status = ERROR_CREATE_ZIP;
    }

    int size_buf = WRITE_BUFFER_SIZE;
    Bytef* buf = (Bytef*) malloc(size_buf);

    // Get information about the file on disk so we can store it in zip
    zip_fileinfo zi = { 0 };
    getFileTime(filename, &zi.tmz_date, &zi.dosDate);

    unsigned long crcFile = 0;
    if (status == ZIP_OK) {
        status = getCRC32(filename, buf, size_buf, &crcFile);
    }

    int zip64 = isLargeFile(filename);

    // Construct the filename that our file will be stored in the zip as.
    const char *savefilenameinzip = filename;
    {
        const char *tmpptr = NULL;
        const char *lastslash = 0;

        for (tmpptr = savefilenameinzip; *tmpptr; tmpptr++) {
            if (*tmpptr == '\\' || *tmpptr == '/') {
                lastslash = tmpptr;
            }
        }
        if (lastslash != NULL) {
            savefilenameinzip = lastslash + 1;
        }
    }

    // Create zip file
    status = zipOpenNewFileInZip3_64(zf, savefilenameinzip, &zi, NULL, 0, NULL, 0, NULL,
    		(opt_compress_level != 0) ? Z_DEFLATED : 0, opt_compress_level, 0,
            -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY, password, crcFile, zip64);

    // Add file to zip
    FILE *fin = NULL;
    if (status == ZIP_OK) {
        fin = fopen64(filename, "rb");
        if (fin == NULL) {
        	status = ERROR_FILE_NOT_FOUND;
        }
    }

    int size_read = 0;
    if (status == ZIP_OK) {
        // Read contents of file and write it to zip
        do {
            size_read = (int) fread(buf, 1, size_buf, fin);
            if ((size_read < size_buf) && (feof(fin) == 0)) {
                status = ERROR_WHILE_READ;
            }

            if (size_read > 0) {
                status = zipWriteInFileInZip(zf, buf, size_read);
            }
        } while ((status == ZIP_OK) && (size_read > 0));
    }

    if (fin) {
    	fclose(fin);
    }

    if (status >= 0) {
        status = zipCloseFileInZip(zf);
    }

    zipClose(zf, NULL);

    // Release memory
    free(buf);
    env->ReleaseStringUTFChars(zipfileStr, zipfilename);
    env->ReleaseStringUTFChars(filenameStr, filename);
    env->ReleaseStringUTFChars(passwordStr, password);

    return status;
}

JNIEXPORT jint JNICALL Java_cvs_com_ziparchiver_minizip_MiniZipWrapper_extractZip(
  JNIEnv * env, jobject, jstring zipfileStr, jstring dirnameStr, jstring passwordStr) {

    jboolean isCopy;
    const char * zipfilename = env->GetStringUTFChars(zipfileStr, &isCopy);
    const char * dirname = env->GetStringUTFChars(dirnameStr, &isCopy);
    const char * password = env->GetStringUTFChars(passwordStr, &isCopy);

    int zipStatus = 0;

    unzFile uf = NULL;

    // Open zip file
    if (zipfilename != NULL) {
        uf = unzOpen64(zipfilename);
    }
    if (uf == NULL) {
    	return ERROR_ZIP_FILE_NOT_FOUND;
    }

    // Extract all
    zipStatus = unzGoToFirstFile(uf);
    if (zipStatus != UNZ_OK) {
    	return ERROR_ZIP_FILE;
    }

    int status = 0;
    status += extractCurrentFile(uf, dirname, password);

    while(zipStatus == UNZ_OK) {
        zipStatus = unzGoToNextFile(uf);
        if(zipStatus == UNZ_OK) {
            status += extractCurrentFile(uf, dirname, password);
        }
    }

    // Release memory
    env->ReleaseStringUTFChars(zipfileStr, zipfilename);
    env->ReleaseStringUTFChars(dirnameStr, dirname);
    env->ReleaseStringUTFChars(passwordStr, password);

    return status;
}

JNIEXPORT jstring JNICALL Java_cvs_com_ziparchiver_minizip_MiniZipWrapper_getFilenameInZip(
  JNIEnv * env, jobject, jstring zipfileStr) {

	jboolean isCopy;
	const char * zipfilename = env->GetStringUTFChars(zipfileStr, &isCopy);

	// Open zip file
	unzFile uf = NULL;
	if (zipfilename != NULL) {
		uf = unzOpen64(zipfilename);
	}
	if (uf == NULL) {
		return NULL;
	}

	// Get filename in zip
	unz_file_info64 file_info = { 0 };
	char filename_in_zip[MAX_FILENAME_LEN] = { 0 };

	int status = unzGetCurrentFileInfo64(uf, &file_info, filename_in_zip, sizeof(filename_in_zip), NULL, 0, NULL, 0);
	if (status != UNZ_OK) {
		return NULL;
	}

	jstring result = env->NewStringUTF((const char*) &filename_in_zip);

	// Release memory
	env->ReleaseStringUTFChars(zipfileStr, zipfilename);

	return result;
}
