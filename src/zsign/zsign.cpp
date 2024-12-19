#include "../Headers/zsign.h"

const struct option options[] = {
    {"pkey", required_argument, NULL, 'p'},
    {"prov", required_argument, NULL, 'm'},
    {"output", required_argument, NULL, 'o'},
    {"password", required_argument, NULL, 'P'},
    {"bundle_id", required_argument, NULL, 'b'},
    {"bundle_name", required_argument, NULL, 'n'},
    {"bundle_version", required_argument, NULL, 'v'},
    {"zip_level", required_argument, NULL, 'z'},
    {"dylib", required_argument, NULL, 'l'},
    {"entitlements", required_argument, NULL, 'E'},
    {"remove_embedded", no_argument, NULL, 'e'},
    {"remove_watch", no_argument, NULL, 'r'},
    {"debug", no_argument, NULL, 'd'},
    {"force", no_argument, NULL, 'f'},
    {"weak", no_argument, NULL, 'w'},
    {"quiet", no_argument, NULL, 'q'},
    {"version", no_argument, NULL, 'V'},
    {"help", no_argument, NULL, 'h'},
};

int usage()
{
    ZLog::Print("Usage: zsign [-bnvledfwrqVh] [-p privkey.p12/pem] [-P p12_pass] [-m mobile.provision] [-z compression_level] unsigned.ipa [-o signed.ipa] \n");
    ZLog::Print("Options:\n");
    ZLog::Print("-p, --pkey\t\tPath to private key or p12 file. (PEM or DER format)\n");
    ZLog::Print("-m, --prov\t\tPath to provisioning profile.\n");
    ZLog::Print("-o, --output\t\tPath to output ipa file.\n");
    ZLog::Print("-P, --password\t\tPassword for private key or p12 file.\n");
    ZLog::Print("-b, --bundle_id\t\tNew bundle id to change.\n");
    ZLog::Print("-n, --bundle_name\tNew bundle name to change.\n");
    ZLog::Print("-v, --bundle_version\tNew bundle version to change.\n");
    ZLog::Print("-z, --zip_level\t\tCompressed level when output the ipa file. (0-9)\n");
    ZLog::Print("-l, --dylib\t\tPath to inject dylib file.\n");
    ZLog::Print("-E, --entitlements\tPath to entitlements file.\n");
    ZLog::Print("-e, --remove_embedded\tRemove embedded.mobileprovision.\n");
    ZLog::Print("-r, --remove_watch\tRemove WatchOS app from the package.\n");
    ZLog::Print("-d, --debug\t\tGenerate debug output files. (.zsign_debug folder)\n");
    ZLog::Print("-f, --force\t\tForce sign without cache when signing folder.\n");
    ZLog::Print("-w, --weak\t\tInject dylib as LC_LOAD_WEAK_DYLIB.\n");
    ZLog::Print("-q, --quiet\t\tQuiet operation.\n");
    ZLog::Print("-V, --version\t\tShow version.\n");
    ZLog::Print("-h, --help\t\tShow this message.\n");
    return 0;
}

std::string getCacheDirectory()
{
    const char *homeDir = getenv("HOME");
    if (homeDir == nullptr)
    {
        std::cerr << "HOME environment variable is not set." << std::endl;
        return "";
    }

    std::string cacheDir = std::string(homeDir) + "/.zsign";

    struct stat st;
    if (stat(cacheDir.c_str(), &st) != 0)
    {
        if (mkdir(cacheDir.c_str(), 0700) != 0)
        {
            std::cerr << "Error creating cache directory: " << cacheDir << std::endl;
            return "";
        }
    }
    else if (!S_ISDIR(st.st_mode))
    {
        std::cerr << "Path exists but is not a directory: " << cacheDir << std::endl;
        return "";
    }
    return cacheDir;
}

int main(int argc, char *argv[])
{
    ZTimer gtimer;
    bool bForce = false;
    bool bWeakInject = false;
    bool bRemoveEmbedded = false;
    bool bRemoveWatch = false;
    uint32_t uZipLevel = 0;
    string strCertFile;
    string strPKeyFile;
    string strProvFile;
    string strPassword;
    string strBundleId;
    string strBundleVersion_Short;
    string strBundleVersion_Long;
    string strDyLibFile;
    string strOutputFile;
    string strDisplayName;
    string strEntitlementsFile;

    int opt = 0;
    int argslot = -1;
    opterr = 0;

    const char *optstring = "p:m:o:P:b:n:v:z:l:E:erdfwqVh";

    while (-1 != (opt = getopt_long(argc, argv, optstring, options, &argslot)))
    {
        switch (opt)
        {
        case 'p':
            strPKeyFile = optarg;
            break;
        case 'm':
            strProvFile = optarg;
            break;
        case 'o':
            strOutputFile = GetCanonicalizePath(optarg);
            break;
        case 'P':
            strPassword = optarg;
            break;
        case 'b':
            strBundleId = optarg;
            break;
        case 'n':
            strDisplayName = optarg;
            break;
        case 'v':
            strBundleVersion_Short = optarg;
            strBundleVersion_Long = optarg;
            break;
        case 'z':
            uZipLevel = atoi(optarg);
            break;
        case 'l':
            strDyLibFile = optarg;
            break;
        case 'E':
            strEntitlementsFile = optarg;
            break;
        case 'e':
            bRemoveEmbedded = true;
            break;
        case 'd':
            ZLog::SetLogLever(ZLog::E_DEBUG);
            break;
        case 'f':
            bForce = true;
            break;
        case 'w':
            bWeakInject = true;
            break;
        case 'r':
            bRemoveWatch = true;
            break;
        case 'q':
            ZLog::SetLogLever(ZLog::E_NONE);
            break;
        case 'V':
            printf("Version: 0.5.7\n");
            return 0;
        case 'h':
            return usage();
        case '?':
            return usage();
        default:
            printf("Unknown option: %c\n", opt);
            return usage();
        }
        ZLog::DebugV("Option:\t-%c, %s\n", opt, optarg ? optarg : "");
    }

    if (optind >= argc)
    {
        return usage();
    }

    if (ZLog::IsDebug())
    {
        CreateFolder("./.zsign_debug");
        for (int i = optind; i < argc; i++)
        {
            ZLog::DebugV("Argument:\t%s\n", argv[i]);
        }
    }

    string strPath = GetCanonicalizePath(argv[optind]);
    if (!IsFileExists(strPath.c_str()))
    {
        ZLog::ErrorV("Invalid Path! %s\n", strPath.c_str());
        return -1;
    }

    bool bZipFile = false;
    if (!IsFolder(strPath.c_str()))
    {
        bZipFile = IsZipFile(strPath.c_str());
        if (!bZipFile)
        {
            ZMachO macho;
            if (macho.Init(strPath.c_str()))
            {
                if (!strDyLibFile.empty())
                {
                    bool bCreate = false;
                    macho.InjectDyLib(bWeakInject, strDyLibFile.c_str(), bCreate);
                }
                else
                {
                    macho.PrintInfo();
                }
                macho.Free();
            }
            return 0;
        }
    }

    ZTimer timer;
    ZSignAsset zSignAsset;
    if (!zSignAsset.Init(strCertFile, strPKeyFile, strProvFile, strEntitlementsFile, strPassword))
    {
        return -1;
    }

    bool bEnableCache = true;
    string strFolder = strPath;

    if (bZipFile)
    {
        bForce = true;
        bEnableCache = false;
        StringFormat(strFolder, "%s/zsign_folder_%llu", getCacheDirectory().c_str(), timer.Reset());
        ZLog::PrintV("Extracting: %s (%s) -> %s\n", strPath.c_str(), GetFileSizeString(strPath.c_str()).c_str(), strFolder.c_str());
        RemoveFolder(strFolder.c_str());
        if (!SystemExec("7z x \"%s\" -y -o\"%s\" -bb0", strPath.c_str(), strFolder.c_str()))
        {
            RemoveFolder(strFolder.c_str());
            ZLog::ErrorV("Extract Failed!\n");
            return -1;
        }
        timer.PrintResult(true, "Extract OK");
    }

    timer.Reset();
    ZAppBundle bundle;
    bool bRet = bundle.SignFolder(&zSignAsset, strFolder, strBundleId, strBundleVersion_Short, strBundleVersion_Long, strDisplayName, strDyLibFile, bForce, bWeakInject, bEnableCache, bRemoveEmbedded, bRemoveWatch);
    timer.PrintResult(bRet, "Signed %s!", bRet ? "OK" : "Failed");

    if (strOutputFile.empty())
    {
        StringFormat(strOutputFile, "%s/zsign_temp_%llu.ipa", getCacheDirectory().c_str(), GetMicroSecond());
    }

    if (!strOutputFile.empty())
    {
        timer.Reset();
        size_t pos = bundle.m_strAppFolder.rfind("/Payload");
        if (string::npos == pos)
        {
            ZLog::Error("Can't Find Payload Directory!\n");
            return -1;
        }

        ZLog::PrintV("Compressing: %s\n", strOutputFile.c_str());
        string strBaseFolder = bundle.m_strAppFolder.substr(0, pos);
        char szOldFolder[PATH_MAX] = {0};
        if (NULL != getcwd(szOldFolder, PATH_MAX))
        {
            if (0 == chdir(strBaseFolder.c_str()))
            {
                uZipLevel = uZipLevel > 9 ? 9 : uZipLevel;
                RemoveFile(strOutputFile.c_str());
                SystemExec("7z a -tzip -mx=%u -y \"%s\" Payload -bb0", uZipLevel, strOutputFile.c_str());
                chdir(szOldFolder);
                if (!IsFileExists(strOutputFile.c_str()))
                {
                    ZLog::Error("Compress Failed!\n");
                    return -1;
                }
            }
        }
        timer.PrintResult(true, "Compress OK! (%s)", GetFileSizeString(strOutputFile.c_str()).c_str());
    }

    string tempPattern = getCacheDirectory() + "/zsign_temp_";
    string folderPattern = getCacheDirectory() + "/zsign_folder_";
    if (0 == strOutputFile.find(tempPattern))
    {
        RemoveFile(strOutputFile.c_str());
    }
    if (0 == strFolder.find(folderPattern))
    {
        RemoveFolder(strFolder.c_str());
    }

    gtimer.Print("Success!");
    return bRet ? 0 : -1;
}