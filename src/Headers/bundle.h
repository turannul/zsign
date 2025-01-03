#pragma once
#include "common.h"
#include "json.h"
#include "base64.h"
#include "openssl.h"
#include "macho.h"
#include "sys/types.h"
#include "sys/stat.h"
#include <vector>

class ZAppBundle
{
public:
	ZAppBundle();

public:
	bool SignFolder(ZSignAsset *pSignAsset, const string &strFolder, const string &strBundleID, const string &strBundleVersion_Short, const string &strBundleVersion_Long, const string &strDisplayName, const string &strDyLibFile, bool bForce, bool bWeakInject, bool bRemoveEmbedded, bool bRemmoveWatch, bool bEnableCache);

private:
	bool SignNode(JValue &jvNode);
	void GetNodeChangedFiles(JValue &jvNode);
	void GetChangedFiles(JValue &jvNode, vector<string> &arrChangedFiles);
	void GetPlugIns(const string &strFolder, vector<string> &arrPlugIns);

private:
	bool FindAppFolder(const string &strFolder, string &strAppFolder);
	bool GetObjectsToSign(const string &strFolder, JValue &jvInfo);
	bool GetSignFolderInfo(const string &strFolder, JValue &jvNode, bool bGetName = false);

private:
	bool GenerateCodeResources(const string &strFolder, JValue &jvCodeRes);
	void GetFolderFiles(const string &strFolder, const string &strBaseFolder, set<string> &setFiles);

private:
	bool m_bForceSign;
	bool m_bWeakInject;
	bool m_bRemoveEmbedded;
	bool m_bRemoveWatch;
	string m_strDyLibPath;
	ZSignAsset *m_pSignAsset;

public:
	string m_strAppFolder;
};
