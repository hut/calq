#include "FileIO.h"

#ifdef OPENSSL
#include <openssl/hmac.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>
#endif

shared_ptr<File> File::Open (const string &path, const char *mode) 
{
    if (IsWeb(path)) {
        return make_shared<WebFile>(path, mode);
    } else {
        return make_shared<File>(path, mode);
    }
}

bool File::Exists (const string &path)
{
    bool result = false;
    if (IsWeb(path)) {
        CURL *ch = curl_easy_init();
        curl_easy_setopt(ch, CURLOPT_NOBODY, 1);
        curl_easy_setopt(ch, CURLOPT_FAILONERROR, 1);

        string url = path;
        if (IsS3(url))
            url = GetURLforS3(url, ch, "HEAD");
        curl_easy_setopt(ch, CURLOPT_URL, url.c_str());

        result = (curl_easy_perform(ch) == CURLE_OK);
        curl_easy_cleanup(ch);
    } else {
        FILE *f = fopen(path.c_str(), "r");
        result = (f != 0);
        if (f) fclose(f);
    }
    return result;
}

string File::FullPath (const string &s) 
{
    if (IsWeb(s))
        return s;
    char *pp = realpath(s.c_str(), 0);
    string p = pp;
    free(pp);
    return p;
}

string File::RemoveExtension (const string &s) 
{
    int i = s.find_last_of(".");
    if (i == string::npos) 
        i = s.size();
    return s.substr(0, i);
}

bool File::IsWeb (const string &path)
{
    return path.find("://") != string::npos;
}

bool File::IsS3 (const string &path)
{
    return IsWeb(path) && (path.size() > 5 && path.substr(0, 5) == "s3://");
}

string File::GetURLforS3 (string url, CURL *ch, string method) 
{
    if (!IsS3(url))
        return url;
    
    url = url.substr(5);
    auto pos = url.find('/');
    if (pos == string::npos)
        return url; 

    string bucket = url.substr(0, pos);
    string location = url.substr(pos + 1);

    url = S("https://%s.s3.amazonaws.com/%s", bucket.c_str(), location.c_str());
    //LOG("Using %s", url.c_str());
    #ifdef OPENSSL
        char *accessKey = getenv("AWS_ACCESS_KEY_ID");
        char *secretKey = getenv("AWS_SECRET_ACCESS_KEY");
        if (accessKey && secretKey && string(accessKey) != "" && string(secretKey) != "") {
            curl_slist *list = 0;
            //LOG("Using AWS credentials found in AWS_ACCESS_KEY_ID and AWS_SECRET_ACCESS_KEY");
            
            list = curl_slist_append(list, "Accept:");
            list = curl_slist_append(list, S("Host: %s.s3.amazonaws.com", bucket.c_str()).c_str());

            // Proper  AWS date
            time_t rawtime;
            time(&rawtime);
            tm *tms = localtime(&rawtime);
            char date[150];
            strftime(date, 150, "%a, %d %b %Y %T %z", tms);
            list = curl_slist_append(list, S("Date: %s", date).c_str());

            // HMAC-SHA1 hash
            string data = S("%s\n\n\n%s\n/%s/%s", method.c_str(), date, bucket.c_str(), location.c_str());
            unsigned char* digest = HMAC(
                EVP_sha1(), 
                secretKey, strlen(secretKey), 
                (unsigned char*)data.c_str(), data.size(), 
                NULL, NULL
            );    

            // Base64 
            BUF_MEM *bufferPtr;
            BIO *b64 = BIO_new(BIO_f_base64());
            BIO *bio = BIO_new(BIO_s_mem());
            bio = BIO_push(b64, bio);
            BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL); //Ignore newlines - write everything in one line
            BIO_write(bio, digest, 20);
            BIO_flush(bio);
            BIO_get_mem_ptr(bio, &bufferPtr);
            BIO_set_close(bio, BIO_NOCLOSE);
            BIO_free_all(bio);

            char *encodedSecret = (*bufferPtr).data;

            //auto q =S("curl -X GET -H 'Host: 1000genomes.s3.amazonaws.com' -H 'Date: %s' -H 'Authorization: AWS %s:%s' %s",
            //  date, accessKey, encodedSecret, url.c_str());
            //LOG("%s",q.c_str());
            //system(q.c_str());

            list = curl_slist_append(list, S("Authorization: AWS %s:%s", accessKey, encodedSecret).c_str());
            curl_easy_setopt(ch, CURLOPT_HTTPHEADER, list);
        }
    #endif
    return url;
}

/*************************************************************************/

File::File () 
{
    fh = 0;
}

File::File (FILE *handle) 
{
    fh = handle;
}

File::File (const string &path, const char *mode) 
{ 
    open(path, mode); 
}

File::~File () 
{
    close();
}

void File::open (const string &path, const char *mode) 
{ 
    fh = fopen(path.c_str(), mode); 
    if (!fh) throw DZException("Cannot open file %s", path.c_str());
    get_size();
}

void File::close () 
{ 
    if (fh) fclose(fh);
    fh = 0; 
}

ssize_t File::read (void *buffer, size_t size) 
{ 
    return fread(buffer, 1, size, fh); 
}

ssize_t File::read (void *buffer, size_t size, size_t offset) 
{
    fseek(fh, offset, SEEK_SET);
    return read(buffer, size);
}

size_t File::advance(size_t size)
{
    fseek(fh, size, SEEK_CUR);
    return ftell(fh);
}   

char File::getc ()
{
    char c;
    if (read(&c, 1) == 0)
        c = EOF;
    return c;
}

uint8_t File::readU8 () 
{
    uint8_t var;
    if (read(&var, sizeof(uint8_t)) != sizeof(uint8_t))
        throw DZException("uint8_t read failed");
    return var;
}

uint16_t File::readU16 () 
{
    uint16_t var;
    if (read(&var, sizeof(uint16_t)) != sizeof(uint16_t))
        throw DZException("uint16_t read failed");
    return var;
}

uint32_t File::readU32 () 
{
    uint32_t var;
    if (read(&var, sizeof(uint32_t)) != sizeof(uint32_t))
        throw DZException("uint32_t read failed");
    return var;
}

uint64_t File::readU64 () 
{
    uint64_t var;
    if (read(&var, sizeof(uint64_t)) != sizeof(uint64_t))
        throw DZException("uint64_t read failed");
    return var;
}

ssize_t File::write (void *buffer, size_t size) 
{ 
    return fwrite(buffer, 1, size, fh); 
}

ssize_t File::tell ()
{
    return ftell(fh);
}

ssize_t File::seek (size_t pos)
{
    return fseek(fh, pos, SEEK_SET);
}

size_t File::size () 
{
    return fsize;
}

bool File::eof () 
{ 
    return feof(fh); 
}

void File::get_size () 
{
    fseek(fh, 0, SEEK_END);
    fsize = ftell(fh);
    fseek(fh, 0, SEEK_SET);
}

void *File::handle ()
{
    return fh;
}

WebFile::WebFile (const string &path, const char *mode) 
{ 
    open(path, mode); 
}

WebFile::~WebFile () 
{
    close();
}

void WebFile::open (const string &path, const char *mode) 
{ 
    ch = curl_easy_init();
    if (!ch) throw DZException("Cannot open file %s", path.c_str());

    string url = path;
    if (IsS3(url))
        url = GetURLforS3(url, ch);

    curl_easy_setopt(ch, CURLOPT_URL, url.c_str());
    curl_easy_setopt(ch, CURLOPT_VERBOSE, optLogLevel >= 2);
    curl_easy_setopt(ch, CURLOPT_WRITEFUNCTION, WebFile::CURLCallback); 
    curl_easy_setopt(ch, CURLOPT_FAILONERROR, 1);
    get_size();
    foffset = 0;
}

void WebFile::close () 
{ 
    if (ch) curl_easy_cleanup(ch);
    ch = 0; 
}

ssize_t WebFile::read (void *buffer, size_t size) 
{ 
    return read(buffer, size, foffset);
}

ssize_t WebFile::read(void *buffer, size_t size, size_t offset) 
{
    CURLBuffer bfr;
    curl_easy_setopt(ch, CURLOPT_WRITEDATA, (void*)&bfr); 
    curl_easy_setopt(ch, CURLOPT_RANGE, S("%zu-%zu", offset, offset + size - 1).c_str());
    auto result = curl_easy_perform(ch);
    if (result != CURLE_OK)
        throw DZException("CURL read failed");
    memcpy(buffer, bfr.data, bfr.size);
    foffset = offset + bfr.size;
    return bfr.size;
}

ssize_t WebFile::write (void *buffer, size_t size) 
{ 
    throw DZException("CURL write is not supported"); 
}

ssize_t WebFile::tell ()
{
    return foffset;
}

ssize_t WebFile::seek (size_t pos)
{
    return (foffset = pos);
}

size_t WebFile::size () 
{
    return fsize;
}

bool WebFile::eof () 
{ 
    return foffset >= fsize;  
}

void WebFile::get_size () 
{
    curl_easy_setopt(ch, CURLOPT_NOBODY, 1L);
    curl_easy_setopt(ch, CURLOPT_HEADER, 0L);
    curl_easy_perform(ch);
 
    double sz;
    auto res = curl_easy_getinfo(ch, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &sz);
    if (res != CURLE_OK) 
        throw DZException("Web server does not support file size query");
    fsize = (size_t)sz;
    curl_easy_setopt(ch, CURLOPT_NOBODY, 0);
}

shared_ptr<File> WebFile::Download (const string &path, bool detectGZFiles)
{
    LOGN("Downloading %s ...     ", path.c_str());
    CURL *ch = curl_easy_init();
    string url = path;
    if (IsS3(url))
        url = GetURLforS3(url, ch);
    curl_easy_setopt(ch, CURLOPT_URL, url.c_str());
    curl_easy_setopt(ch, CURLOPT_VERBOSE, optLogLevel >= 2);
    curl_easy_setopt(ch, CURLOPT_WRITEFUNCTION, WebFile::CURLDownloadCallback); 
    FILE *f = tmpfile(); //fopen("HAMO.txt","a+b");// tmpfile();
    curl_easy_setopt(ch, CURLOPT_WRITEDATA, f);
    curl_easy_setopt(ch, CURLOPT_NOPROGRESS, 0);
    curl_easy_setopt(ch, CURLOPT_FAILONERROR, 1);
    curl_easy_setopt(ch, CURLOPT_PROGRESSFUNCTION, WebFile::CURLDownloadProgressCallback);
    auto res = curl_easy_perform(ch);
    if (res != CURLE_OK) 
        throw DZException("Cannot download file %s", path.c_str());
    curl_easy_cleanup(ch);
    fflush(f);
    fseek(f, 0, SEEK_SET);
    unsigned char magic[2];
    fread(magic, 1, 2, f);
    fseek(f, 0, SEEK_SET);
    fflush(f);

    if (detectGZFiles && magic[0] == 0x1f && magic[1] == 0x8b) {
        LOG("\b\b\b\b opened as GZ file"); 
        return make_shared<GzFile>(f);
    } else {
        LOG("");
        return make_shared<File>(f);
    }
}

int WebFile::CURLDownloadProgressCallback (void* ptr, double TotalToDownload, double NowDownloaded, double TotalToUpload, double NowUploaded)
{
    if (int(TotalToDownload))
        LOGN("\b\b\b\b%3.0lf%%", 100.0 * NowDownloaded / TotalToDownload);
    return 0;
}

size_t WebFile::CURLDownloadCallback (void *ptr, size_t size, size_t nmemb, FILE *stream) 
{
    return fwrite(ptr, size, nmemb, stream);
}

size_t WebFile::CURLCallback (void *ptr, size_t size, size_t nmemb, void *data) 
{
    size_t realsize = size * nmemb;
    CURLBuffer *buffer = (CURLBuffer*)data;
    buffer->data = (char*)realloc(buffer->data, buffer->size + realsize);
    if (!buffer->data) 
        throw DZException("CURL reallocation failed");
    memcpy(buffer->data + buffer->size, ptr, realsize);
    buffer->size += realsize;       
    return realsize;
}

void *WebFile::handle ()
{
    return ch;
}

GzFile::GzFile (const string &path, const char *mode) 
{ 
    open(path, mode); 
}

GzFile::GzFile (FILE *handle) 
{
    fh = gzdopen(fileno(handle), "rb");
    if (!fh) throw DZException("Cannot open GZ file via handle");
}

GzFile::~GzFile () 
{
    close();
}

void GzFile::open (const string &path, const char *mode) 
{ 
    fh = gzopen(path.c_str(), mode); 
    if (!fh) throw DZException("Cannot open file %s", path.c_str());
}

void GzFile::close () 
{ 
    if (fh) gzclose(fh);
    fh = 0; 
}

ssize_t GzFile::read (void *buffer, size_t size) 
{ 
    const size_t offset = 1 * (size_t)GB;
    if (size > offset) {
        return gzread(fh, buffer, offset) + read((char*)buffer + offset, size - offset); 
    } else {
        return gzread(fh, buffer, size);
    }
}

ssize_t GzFile::read(void *buffer, size_t size, size_t offset) 
{
    throw DZException("GZ random access is not yet supported");
}

ssize_t GzFile::write (void *buffer, size_t size) 
{ 
    return gzwrite(fh, buffer, size); 
}

ssize_t GzFile::tell ()
{
    return gztell(fh);
}

ssize_t GzFile::seek (size_t pos)
{
    return gzseek(fh, pos, SEEK_SET);
}

size_t GzFile::size () 
{
    throw DZException("GZ file size is not supported");
}

bool GzFile::eof () 
{ 
    return gzeof(fh); 
}

void GzFile::get_size () 
{
    throw DZException("GZ file size is not supported");
}

void *GzFile::handle ()
{
    return fh;
}