#include "../Headers/zsign.h"

const struct option options[] = {
	// k:m:c:o:p:b:n:r:z:l:dfewqvh
    {"pkey", required_argument, NULL, 'k'},
    {"prov", required_argument, NULL, 'm'},
    {"cert", required_argument, NULL, 'c'},
    {"debug", no_argument, NULL, 'd'},
    {"force", no_argument, NULL, 'f'},
    {"output", required_argument, NULL, 'o'},
    {"password", required_argument, NULL, 'p'},
    {"bundle_id", required_argument, NULL, 'b'},
    {"bundle_name", required_argument, NULL, 'n'},
    {"bundle_version", required_argument, NULL, 'r'},
    {"remove_embedded", no_argument, NULL, 'e'},
    {"zip_level", required_argument, NULL, 'z'},
    {"dylib", required_argument, NULL, 'l'},
    {"weak", no_argument, NULL, 'w'},
    {"quiet", no_argument, NULL, 'q'},
	{"version", no_argument, NULL, 'v'},
    {"help", no_argument, NULL, 'h'},
};

int usage()
{
    ZLog::Print("Usage: zsign [-qfde] [-k privkey.p12 & privkey.pem] [-m mobile.provision] [-o signed.ipa] unsigned.ipa\n");
    ZLog::Print("Options:\n");
    ZLog::Print("-k, --pkey\t\tPath to private key or p12 file. (PEM or DER format)\n");
    ZLog::Print("-m, --prov\t\tPath to provisioning profile.\n");
    ZLog::Print("-c, --cert\t\tPath to certificate file. (PEM or DER format)\n");
    ZLog::Print("-d, --debug\t\tGenerate debug output files. (.zsign_debug folder)\n");
    ZLog::Print("-f, --force\t\tForce sign without cache when signing folder.\n");
    ZLog::Print("-o, --output\t\tPath to output ipa file.\n");
    ZLog::Print("-p, --password\t\tPassword for private key or p12 file.\n");
    ZLog::Print("-b, --bundle_id\t\tNew bundle id to change.\n");
    ZLog::Print("-n, --bundle_name\tNew bundle name to change.\n");
    ZLog::Print("-r, --bundle_version\tNew bundle version to change.\n");
    ZLog::Print("-e, --remove_embedded\tRemove emmbedded.mobileprovision.\n");
    ZLog::Print("-z, --zip_level\t\tCompressed level when output the ipa file. (0-9)\n");
    ZLog::Print("-l, --dylib\t\tPath to inject dylib file.\n");
    ZLog::Print("-w, --weak\t\tInject dylib as LC_LOAD_WEAK_DYLIB.\n");
    ZLog::Print("-q, --quiet\t\tQuiet operation.\n");
    ZLog::Print("-v, --version\t\tShow version.\n");
    ZLog::Print("-h, --help\t\tShow this message.\n");
    ZLog::Print("Modified for FavourSign by Turann_");
    return 0;
}

int main(int argc, char *argv[])
{
    ZTimer gtimer;

    bool bForce = false;
    bool bWeakInject = false;
	bool bRemoveEmbedded = false;
    uint32_t uZipLevel = 0;
    string strCertFile;
    string strPKeyFile;
    string strProvFile;
    string strPassword;
    string strBundleId;
    string strBundleVersion;
    string intRequiredOSVersion;
    string strDyLibFile;
    string strOutputFile;
    string strDisplayName;

    int opt = 0;
    int argslot = -1;
	opterr = 0;
    while (-1 != (opt = getopt_long(argc, argv, "k:m:c:o:p:b:n:r:z:l:dfewqvh", options, &argslot))) {
        switch (opt) {
        case 'd':
            ZLog::SetLogLever(ZLog::E_DEBUG);
            break;
        case 'f':
            bForce = true;
            break;
        case 'c':
            strCertFile = optarg;
            break;
        case 'k':
            strPKeyFile = optarg;
            break;
        case 'm':
            strProvFile = optarg;
            break;
        case 'p':
            strPassword = optarg;
            break;
        case 'b':
            strBundleId = optarg;
            break;
        case 'r':
            strBundleVersion = optarg;
            break;
        case 'n':
            strDisplayName = optarg;
            break;
        case 'e':
            bRemoveEmbedded = true;
            break;
        case 'l':
            strDyLibFile = optarg;
            break;
        case 'o':
            strOutputFile = GetCanonicalizePath(optarg);
            break;
        case 'z':
            uZipLevel = atoi(optarg);
            break;
        case 'w':
            bWeakInject = true;
            break;
        case 'q':
            ZLog::SetLogLever(ZLog::E_NONE);
            break;
        case 'v': {
            printf("version: 0.5.5\n");
            return 0;
        }
        case 'h':
            return usage();
			break;
		default:
			printf("Unknown option.");
			return 1;
		}
        ZLog::DebugV("Option:\t-%c, %s\n", opt, optarg);
    }

    if (optind >= argc) { return usage(); }

    if (ZLog::IsDebug()) {
        CreateFolder("./.zsign_debug");
        for (int i = optind; i < argc; i++) { ZLog::DebugV("Argument:\t%s\n", argv[i]); }
    }

    string strPath = GetCanonicalizePath(argv[optind]);
    if (!IsFileExists(strPath.c_str())) {
        ZLog::ErrorV("Invalid Path! %s\n", strPath.c_str());
        return -1;
    }

    bool bZipFile = false;
    if (!IsFolder(strPath.c_str())) {
        bZipFile = IsZipFile(strPath.c_str());
        if (!bZipFile) { //macho file
            ZMachO macho;
            if (macho.Init(strPath.c_str())) {
                if (!strDyLibFile.empty()) { //inject dylib
                    bool bCreate = false;
                    macho.InjectDyLib(bWeakInject, strDyLibFile.c_str(), bCreate);
                } else {
                    macho.PrintInfo();
                }
                macho.Free();
            }
            return 0;
        }
    }

    ZTimer timer;
    ZSignAsset zSignAsset;
    if (!zSignAsset.Init(strCertFile, strPKeyFile, strProvFile, strPassword)) { return -1; }

    bool bEnableCache = true;
    string strFolder = strPath;
    // True if it is an ipa file
    if (bZipFile) {
        bForce = true;
        bEnableCache = false;
        StringFormat(strFolder, "/var/tmp/zsign_folder_%llu", timer.Reset());
        ZLog::PrintV("Extracting: %s (%s) -> %s\n", strPath.c_str(), GetFileSizeString(strPath.c_str()).c_str(), strFolder.c_str());
        RemoveFolder(strFolder.c_str());
        if (!SystemExec("7z x \"%s\" -y -o\"%s\" -bb0", strPath.c_str(), strFolder.c_str())) {
            RemoveFolder(strFolder.c_str());
            ZLog::ErrorV("Extract Failed!\n");
            return -1;
        }
        timer.PrintResult(true, "Extract OK");
    }

    timer.Reset();
    ZAppBundle bundle;
    bool bRet = bundle.SignFolder(&zSignAsset, strFolder, strBundleId, strBundleVersion, strDisplayName, strDyLibFile, bForce, bWeakInject, bRemoveEmbedded, bEnableCache);
    timer.PrintResult(bRet, "Signed %s!", bRet ? "OK" : "Failed");

    if (strOutputFile.empty()) { StringFormat(strOutputFile, "/var/tmp/zsign_temp_%llu.ipa", GetMicroSecond()); }

    if (!strOutputFile.empty()) {
        timer.Reset();
        size_t pos = bundle.m_strAppFolder.rfind("/Payload");
        if (string::npos == pos) {
            ZLog::Error("Can't Find Payload Directory!\n");
            return -1;
        }

        ZLog::PrintV("Compressing: %s\n", strOutputFile.c_str());
        string strBaseFolder = bundle.m_strAppFolder.substr(0, pos);
        char szOldFolder[PATH_MAX] = {0};
        if (NULL != getcwd(szOldFolder, PATH_MAX)) {
            if (0 == chdir(strBaseFolder.c_str())) {
                uZipLevel = uZipLevel > 9 ? 9 : uZipLevel;
                RemoveFile(strOutputFile.c_str());
                SystemExec("7z a -tzip -mx=%u -y \"%s\" Payload -bb0", uZipLevel, strOutputFile.c_str());
                chdir(szOldFolder);
                if (!IsFileExists(strOutputFile.c_str())) {
                    ZLog::Error("Compress Failed!\n");
                    return -1;
                }
            }
        }
        timer.PrintResult(true, "Compress OK! (%s)", GetFileSizeString(strOutputFile.c_str()).c_str());
    }

    if (0 == strOutputFile.find("/var/tmp/zsign_tmp_")) { RemoveFile(strOutputFile.c_str()); }
    if (0 == strFolder.find("/var/tmp/zsign_folder_")) { RemoveFolder(strFolder.c_str()); }

    gtimer.Print("Success!");
    return bRet ? 0 : -1;
}