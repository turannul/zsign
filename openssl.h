#pragma once
#include "common/json.h"

bool GetCertSubjectCN(const string &strCertData, string &strSubjectCN);
bool GetCMSInfo(uint8_t *pCMSData, uint32_t uCMSLength, JValue &jvOutput);
bool GetCMSContent(const string &strCMSDataInput, string &strContentOutput);
bool GenerateCMS(const string &strSignerCertData, const string &strSignerPKeyData, const string &strCDHashData, const string &strCDHashPlist, string &strCMSOutput);

class ZSignAsset
{
public:
	ZSignAsset();

public:
	bool GenerateCMS(const string &strCDHashData, const string &strCDHashesPlist, const string &strCodeDirectorySlotSHA1, const string &strAltnateCodeDirectorySlot256, string &strCMSOutput);
	/// Initialize ZSignAsset for ad-hoc signing.  Entitlements may be optionally read from \p strEntitlementsFile.
	bool Init(const string &strEntitlementsFile);
	/// Initialize ZSignAsset object.
	bool Init(const string &strSignerCertFile, const string &strSignerPKeyFile, const string &strProvisionFile, const string &strEntitlementsFile, const string &strPassword);

public:
	bool m_bAdhoc; /// If true, carry out ad-hoc signature instead; in that case, `m_strTeamId` and `m_strSubjectCN`, can be empty.
	bool m_bSingleBinary; ///< `true` if signing single binary, i.e. `CS_EXECSEG_MAIN_BINARY` flag shall be set
	bool m_bUseSHA256Only; /// If true, serialize a single CSSLOT_CODEDIRECTORY that uses SHA256; otherwise, use both SHA1 and SHA256 (alternate).

	string m_strTeamId;
	string m_strSubjectCN;
	string m_strProvisionData;
	string m_strEntitlementsData;

private:
	void* m_evpPKey;
	void* m_x509Cert;
};
