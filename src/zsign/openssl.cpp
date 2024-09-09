#include "../Headers/openssl.h"
#include "../Headers/Certs/appleDevCA_Cert.h"
#include "../Headers/Certs/appleDevCA_Cert_G3.h"
#include "../Headers/Certs/appleRoot_CA_Cert.h"

class COpenSSLInit {
public:
    COpenSSLInit() {
#if OPENSSL_VERSION_NUMBER < 0x10100000L
        OpenSSL_add_all_algorithms();
        ERR_load_crypto_strings();
#endif
	};
};

COpenSSLInit g_OpenSSLInit;

bool CMSError()
{
	ERR_print_errors_fp(stderr);
	return false;
}

ASN1_TYPE *_GenerateASN1Type(const string &value)
{
	long errline = -1;
	char *genstr = NULL;
	BIO *ldapbio = BIO_new(BIO_s_mem());
	CONF *cnf = NCONF_new(NULL);

	if (cnf == NULL)
	{
		ZLog::Error("Failed to create new CONF structure. (NCONF_new)\n");
		BIO_free(ldapbio);
		return nullptr;
	}
	string a = "asn1=SEQUENCE:A\n[A]\nC=OBJECT:sha256\nB=FORMAT:HEX,OCT:" + value + "\n";
	int code = BIO_puts(ldapbio, a.c_str());
	if (NCONF_load_bio(cnf, ldapbio, &errline) <= 0)
	{
		BIO_free(ldapbio);
		NCONF_free(cnf);
		ZLog::Error("Failed to load configuration from BIO (NCONF_load_bio)\n");
		return nullptr;
	}
	BIO_free(ldapbio);
	genstr = NCONF_get_string(cnf, "default", "asn1");

	if (genstr == NULL)
	{
		ZLog::Error("Failed to get ASN1 string from CONF structure (NCONF_get_string)\n");
		NCONF_free(cnf);
		return nullptr;
	}
	ASN1_TYPE *ret = ASN1_generate_nconf(genstr, cnf);
	NCONF_free(cnf);
	return ret;
}

bool _GenerateCMS(X509 *scert, EVP_PKEY *spkey, const string &strCDHashData, const string &strCDHashesPlist, const string &strCodeDirectorySlotSHA1, const string &strAltnateCodeDirectorySlot256, string &strCMSOutput)
{
	if (!scert || !spkey)
	{
		return CMSError();
	}

	BIO *bother1;
	unsigned long issuerHash = X509_issuer_name_hash(scert);
	if (0x817d2f7a == issuerHash)
	{
		bother1 = BIO_new_mem_buf(appleDevCACert, (int)strlen(appleDevCACert));
	}
	else if (0x9b16b75c == issuerHash)
	{
		bother1 = BIO_new_mem_buf(appleDevCACertG3, (int)strlen(appleDevCACertG3));
	}
	else
	{
		ZLog::Error("Signer certificate or private key is null\n");
		return CMSError();
	}

	BIO *bother2 = BIO_new_mem_buf(appleRootCACert, (int)strlen(appleRootCACert));
	if (!bother1 || !bother2)
	{
		return CMSError();
	}

	X509 *ocert1 = PEM_read_bio_X509(bother1, NULL, 0, NULL);
	X509 *ocert2 = PEM_read_bio_X509(bother2, NULL, 0, NULL);
	if (!ocert1 || !ocert2)
	{
		return CMSError();
	}

	STACK_OF(X509) *otherCerts = sk_X509_new_null();
	if (!otherCerts)
	{
		return CMSError();
	}

	if (!sk_X509_push(otherCerts, ocert1))
	{
		return CMSError();
	}

	if (!sk_X509_push(otherCerts, ocert2))
	{
		return CMSError();
	}

	BIO *in = BIO_new_mem_buf(strCDHashData.c_str(), (int)strCDHashData.size());
	if (!in)
	{
		return CMSError();
	}

	int nFlags = CMS_PARTIAL | CMS_DETACHED | CMS_NOSMIMECAP | CMS_BINARY;
	CMS_ContentInfo *cms = CMS_sign(NULL, NULL, otherCerts, NULL, nFlags);
	if (!cms)
	{
		return CMSError();
	}

	CMS_SignerInfo *si = CMS_add1_signer(cms, scert, spkey, EVP_sha256(), nFlags);
	//    CMS_add1_signer(cms, NULL, NULL, EVP_sha1(), nFlags);
	if (!si)
	{
		return CMSError();
	}

	// add plist
	ASN1_OBJECT *obj = OBJ_txt2obj("1.2.840.113635.100.9.1", 1);
	if (!obj)
	{
		return CMSError();
	}

	int addHashPlist = CMS_signed_add1_attr_by_OBJ(si, obj, 0x4, strCDHashesPlist.c_str(), (int)strCDHashesPlist.size());

	if (!addHashPlist)
	{
		return CMSError();
	}

	// add CDHashes
	string sha256;
	char buf[16] = {0};
	for (size_t i = 0; i < strAltnateCodeDirectorySlot256.size(); i++)
	{
		sprintf(buf, "%02x", (uint8_t)strAltnateCodeDirectorySlot256[i]);
		sha256 += buf;
	}
	transform(sha256.begin(), sha256.end(), sha256.begin(), ::toupper);

	ASN1_OBJECT *obj2 = OBJ_txt2obj("1.2.840.113635.100.9.2", 1);
	if (!obj2)
	{
		return CMSError();
	}

	X509_ATTRIBUTE *attr = X509_ATTRIBUTE_new();
	X509_ATTRIBUTE_set1_object(attr, obj2);

	ASN1_TYPE *type_256 = _GenerateASN1Type(sha256);
	X509_ATTRIBUTE_set1_data(attr, V_ASN1_SEQUENCE,
							 type_256->value.asn1_string->data, type_256->value.asn1_string->length);
	int addHashSHA = CMS_signed_add1_attr(si, attr);
	if (!addHashSHA)
	{
		return CMSError();
	}

	if (!CMS_final(cms, in, NULL, nFlags))
	{
		return CMSError();
	}

	BIO *out = BIO_new(BIO_s_mem());
	if (!out)
	{
		return CMSError();
	}

	// PEM_write_bio_CMS(out, cms);
	if (!i2d_CMS_bio(out, cms))
	{
		return CMSError();
	}

	BUF_MEM *bptr = NULL;
	BIO_get_mem_ptr(out, &bptr);
	if (!bptr)
	{
		return CMSError();
	}

	strCMSOutput.clear();
	strCMSOutput.append(bptr->data, bptr->length);
	ASN1_TYPE_free(type_256);
	return (!strCMSOutput.empty());
}

bool GenerateCMS(const string &strSignerCertData, const string &strSignerPKeyData, const string &strCDHashData, const string &strCDHashesPlist, string &strCMSOutput)
{
	BIO *bcert = BIO_new_mem_buf(strSignerCertData.c_str(), (int)strSignerCertData.size());
	BIO *bpkey = BIO_new_mem_buf(strSignerPKeyData.c_str(), (int)strSignerPKeyData.size());

	if (!bcert || !bpkey)
	{
		return CMSError();
	}

	X509 *scert = PEM_read_bio_X509(bcert, NULL, 0, NULL);
	EVP_PKEY *spkey = PEM_read_bio_PrivateKey(bpkey, NULL, 0, NULL);
	if (!scert || !spkey)
	{
		ZLog::Error("Failed to create BIO from certificate or private key data\n");
		return CMSError();
	}

	return ::_GenerateCMS(scert, spkey, strCDHashData, strCDHashesPlist, "", "", strCMSOutput);
}

bool GetCMSContent(const string &strCMSDataInput, string &strContentOutput)
{
	if (strCMSDataInput.empty())
	{
		return false;
	}

	BIO *in = BIO_new(BIO_s_mem());
	OPENSSL_assert((size_t)BIO_write(in, strCMSDataInput.data(), (int)strCMSDataInput.size()) == strCMSDataInput.size());
	CMS_ContentInfo *cms = d2i_CMS_bio(in, NULL);
	if (!cms)
	{
		return CMSError();
	}

	ASN1_OCTET_STRING **pos = CMS_get0_content(cms);
	if (!pos)
	{
		return CMSError();
	}

	if (!(*pos))
	{
		return CMSError();
	}

	strContentOutput.clear();
	strContentOutput.append((const char *)(*pos)->data, (*pos)->length);
	return (!strContentOutput.empty());
}

bool GetCertSubjectCN(X509 *cert, string &strSubjectCN)
{
	if (!cert)
	{
		return CMSError();
	}

	X509_NAME *name = X509_get_subject_name(cert);

	int common_name_loc = X509_NAME_get_index_by_NID(name, NID_commonName, -1);
	if (common_name_loc < 0)
	{
		return CMSError();
	}

	X509_NAME_ENTRY *common_name_entry = X509_NAME_get_entry(name, common_name_loc);
	if (common_name_entry == NULL)
	{
		return CMSError();
	}

	ASN1_STRING *common_name_asn1 = X509_NAME_ENTRY_get_data(common_name_entry);
	if (common_name_asn1 == NULL)
	{
		return CMSError();
	}

	strSubjectCN.clear();
	strSubjectCN.append((const char *)common_name_asn1->data, common_name_asn1->length);
	return (!strSubjectCN.empty());
}

bool GetCertSubjectCN(const string &strCertData, string &strSubjectCN)
{
	if (strCertData.empty())
	{
		return false;
	}

	BIO *bcert = BIO_new_mem_buf(strCertData.c_str(), strCertData.size());
	if (!bcert)
	{
		return CMSError();
	}

	X509 *cert = PEM_read_bio_X509(bcert, NULL, 0, NULL);
	if (!cert)
	{
		return CMSError();
	}

	return GetCertSubjectCN(cert, strSubjectCN);
}

void ParseCertSubject(const string &strSubject, JValue &jvSubject)
{
	vector<string> arrNodes;
	StringSplit(strSubject, "/", arrNodes);
	for (size_t i = 0; i < arrNodes.size(); i++)
	{
		vector<string> arrLines;
		StringSplit(arrNodes[i], "=", arrLines);
		if (2 == arrLines.size())
		{
			jvSubject[arrLines[0]] = arrLines[1];
		}
	}
}

#if OPENSSL_VERSION_NUMBER < 0x10100000L
string ASN1_TIMEtoString(ASN1_TIME *time)
{
#else
string ASN1_TIMEtoString(const ASN1_TIME *time)
{
#endif
	BIO *out = BIO_new(BIO_s_mem());
	if (!out)
	{
		CMSError();
		return "";
	}

	ASN1_TIME_print(out, time);
	BUF_MEM *bptr = NULL;
	BIO_get_mem_ptr(out, &bptr);
	if (!bptr)
	{
		CMSError();
		return "";
	}
	string strTime;
	strTime.append(bptr->data, bptr->length);
	return strTime;
}

bool GetCertInfo(X509 *cert, JValue &jvCertInfo)
{
	if (!cert)
	{
		return CMSError();
	}

	jvCertInfo["Version"] = (int)X509_get_version(cert);

	ASN1_INTEGER *asn1_i = X509_get_serialNumber(cert);
	if (asn1_i)
	{
		BIGNUM *bignum = ASN1_INTEGER_to_BN(asn1_i, NULL);
		if (bignum)
		{
			jvCertInfo["SerialNumber"] = BN_bn2hex(bignum);
		}
	}

	jvCertInfo["SignatureAlgorithm"] = OBJ_nid2ln(X509_get_signature_nid(cert));

	EVP_PKEY *pubkey = X509_get_pubkey(cert);
	int type = EVP_PKEY_id(pubkey);
	jvCertInfo["PublicKey"]["Algorithm"] = OBJ_nid2ln(type);

#if OPENSSL_VERSION_NUMBER < 0x10100000L
	jvCertInfo["Validity"]["NotBefore"] = ASN1_TIMEtoString(X509_get_notBefore(cert));
	jvCertInfo["Validity"]["NotAfter"] = ASN1_TIMEtoString(X509_get_notAfter(cert));
#else
	jvCertInfo["Validity"]["NotBefore"] = ASN1_TIMEtoString(X509_get0_notBefore(cert));
	jvCertInfo["Validity"]["NotAfter"] = ASN1_TIMEtoString(X509_get0_notAfter(cert));
#endif

	string strIssuer = X509_NAME_oneline(X509_get_issuer_name(cert), NULL, 0);
	string strSubject = X509_NAME_oneline(X509_get_subject_name(cert), NULL, 0);

	ParseCertSubject(strIssuer, jvCertInfo["Issuer"]);
	ParseCertSubject(strSubject, jvCertInfo["Subject"]);

	return (!strIssuer.empty() && !strSubject.empty());
}

bool GetCMSInfo(uint8_t *pCMSData, uint32_t uCMSLength, JValue &jvOutput)
{
	BIO *in = BIO_new(BIO_s_mem());
	OPENSSL_assert((size_t)BIO_write(in, pCMSData, uCMSLength) == uCMSLength);
	CMS_ContentInfo *cms = d2i_CMS_bio(in, NULL);
	if (!cms)
	{
		return CMSError();
	}

	int detached = CMS_is_detached(cms);
	jvOutput["detached"] = detached;

	const ASN1_OBJECT *obj = CMS_get0_type(cms);
	const char *sn = OBJ_nid2ln(OBJ_obj2nid(obj));
	jvOutput["contentType"] = sn;

	ASN1_OCTET_STRING **pos = CMS_get0_content(cms);
	if (pos)
	{
		if ((*pos))
		{
			ZBase64 b64;
			jvOutput["content"] = b64.Encode((const char *)(*pos)->data, (*pos)->length);
		}
	}

	STACK_OF(X509) *certs = CMS_get1_certs(cms);
	for (int i = 0; i < sk_X509_num(certs); i++)
	{
		JValue jvCertInfo;
		if (GetCertInfo(sk_X509_value(certs, i), jvCertInfo))
		{
			jvOutput["certs"].push_back(jvCertInfo);
		}
	}

	STACK_OF(CMS_SignerInfo) *sis = CMS_get0_SignerInfos(cms);
	for (int i = 0; i < sk_CMS_SignerInfo_num(sis); i++)
	{
		CMS_SignerInfo *si = sk_CMS_SignerInfo_value(sis, i);
		// int CMS_SignerInfo_get0_signer_id(CMS_SignerInfo *si, ASN1_OCTET_STRING **keyid, X509_NAME **issuer, ASN1_INTEGER **sno);

		int nSignedAttsCount = CMS_signed_get_attr_count(si);
		for (int j = 0; j < nSignedAttsCount; j++)
		{
			X509_ATTRIBUTE *attr = CMS_signed_get_attr(si, j);
			if (!attr)
			{
				continue;
			}
			int nCount = X509_ATTRIBUTE_count(attr);
			if (nCount <= 0)
			{
				continue;
			}

			ASN1_OBJECT *obj = X509_ATTRIBUTE_get0_object(attr);
			if (!obj)
			{
				continue;
			}

			char txtobj[128] = {0};
			OBJ_obj2txt(txtobj, 128, obj, 1);

			if (0 == strcmp("1.2.840.113549.1.9.3", txtobj))
			{ // V_ASN1_OBJECT
				ASN1_TYPE *av = X509_ATTRIBUTE_get0_type(attr, 0);
				if (NULL != av)
				{
					jvOutput["attrs"]["ContentType"]["obj"] = txtobj;
					jvOutput["attrs"]["ContentType"]["data"] = OBJ_nid2ln(OBJ_obj2nid(av->value.object));
				}
			}
			else if (0 == strcmp("1.2.840.113549.1.9.4", txtobj))
			{ // V_ASN1_OCTET_STRING
				ASN1_TYPE *av = X509_ATTRIBUTE_get0_type(attr, 0);
				if (NULL != av)
				{
					string strSHASum;
					char buf[16] = {0};
					for (int m = 0; m < av->value.octet_string->length; m++)
					{
						sprintf(buf, "%02x", (uint8_t)av->value.octet_string->data[m]);
						strSHASum += buf;
					}
					jvOutput["attrs"]["MessageDigest"]["obj"] = txtobj;
					jvOutput["attrs"]["MessageDigest"]["data"] = strSHASum;
				}
			}
			else if (0 == strcmp("1.2.840.113549.1.9.5", txtobj))
			{ // V_ASN1_UTCTIME
				ASN1_TYPE *av = X509_ATTRIBUTE_get0_type(attr, 0);
				if (NULL != av)
				{
					BIO *mem = BIO_new(BIO_s_mem());
					ASN1_UTCTIME_print(mem, av->value.utctime);
					BUF_MEM *bptr = NULL;
					BIO_get_mem_ptr(mem, &bptr);
					BIO_set_close(mem, BIO_NOCLOSE);
					string strTime;
					strTime.append(bptr->data, bptr->length);
					BIO_free_all(mem);

					jvOutput["attrs"]["SigningTime"]["obj"] = txtobj;
					jvOutput["attrs"]["SigningTime"]["data"] = strTime;
				}
			}
			else if (0 == strcmp("1.2.840.113635.100.9.2", txtobj))
			{ // V_ASN1_SEQUENCE
				jvOutput["attrs"]["CDHashes2"]["obj"] = txtobj;
				for (int m = 0; m < nCount; m++)
				{
					ASN1_TYPE *av = X509_ATTRIBUTE_get0_type(attr, m);
					if (NULL != av)
					{
						ASN1_STRING *s = av->value.sequence;

						BIO *mem = BIO_new(BIO_s_mem());

						ASN1_parse_dump(mem, s->data, s->length, 2, 0);
						BUF_MEM *bptr = NULL;
						BIO_get_mem_ptr(mem, &bptr);
						BIO_set_close(mem, BIO_NOCLOSE);
						string strData;
						strData.append(bptr->data, bptr->length);
						BIO_free_all(mem);

						string strSHASum;
						size_t pos1 = strData.find("[HEX DUMP]:");
						if (string::npos != pos1)
						{
							size_t pos2 = strData.find("\n", pos1);
							if (string::npos != pos2)
							{
								strSHASum = strData.substr(pos1 + 11, pos2 - pos1 - 11);
							}
						}
						transform(strSHASum.begin(), strSHASum.end(), strSHASum.begin(), ::tolower);
						jvOutput["attrs"]["CDHashes2"]["data"].push_back(strSHASum);
					}
				}
			}
			else if (0 == strcmp("1.2.840.113635.100.9.1", txtobj))
			{ // V_ASN1_OCTET_STRING
				ASN1_TYPE *av = X509_ATTRIBUTE_get0_type(attr, 0);
				if (NULL != av)
				{
					string strPList;
					strPList.append((const char *)av->value.octet_string->data, av->value.octet_string->length);
					jvOutput["attrs"]["CDHashes"]["obj"] = txtobj;
					jvOutput["attrs"]["CDHashes"]["data"] = strPList;
				}
			}
			else
			{
				ASN1_TYPE *av = X509_ATTRIBUTE_get0_type(attr, 0);
				if (NULL != av)
				{
					JValue jvAttr;
					jvAttr["obj"] = txtobj;
					jvAttr["name"] = OBJ_nid2ln(OBJ_obj2nid(obj));
					jvAttr["type"] = av->type;
					jvAttr["count"] = nCount;
					jvOutput["attrs"]["unknown"].push_back(jvAttr);
				}
			}
		}
	}

	return true;
}

ZSignAsset::ZSignAsset()
{
	m_evpPKey = NULL;
	m_x509Cert = NULL;
}

bool ZSignAsset::Init(const string &strSignerCertFile, const string &strSignerPKeyFile, const string &strProvisionFile, const string &strPassword)
{
	ReadFile(strProvisionFile.c_str(), m_strProvisionData);
	if (m_strProvisionData.empty())
	{
		ZLog::Error("Can't Find Provision File!\n");
		return false;
	}

	JValue jvProv;
	string strProvContent;
	if (GetCMSContent(m_strProvisionData, strProvContent))
	{
		if (jvProv.readPList(strProvContent))
		{
			m_strTeamId = jvProv["TeamIdentifier"][0].asCString();
		}
	}

	if (m_strTeamId.empty())
	{
		ZLog::Error("Can't Find TeamId!\n");
		return false;
	}

	X509 *x509Cert = NULL;
	EVP_PKEY *evpPKey = NULL;
	BIO *bioPKey = BIO_new_file(strSignerPKeyFile.c_str(), "r");
	if (NULL != bioPKey)
	{
		evpPKey = PEM_read_bio_PrivateKey(bioPKey, NULL, NULL, (void *)strPassword.c_str());
		if (NULL == evpPKey)
		{
			BIO_reset(bioPKey);
			evpPKey = d2i_PrivateKey_bio(bioPKey, NULL);
			if (NULL == evpPKey)
			{
				BIO_reset(bioPKey);
				OSSL_PROVIDER_load(NULL, "legacy"); // Enable legacy provider on newer versions of OpenSSL
				PKCS12 *p12 = d2i_PKCS12_bio(bioPKey, NULL);
				if (NULL != p12)
				{
					if (0 == PKCS12_parse(p12, strPassword.c_str(), &evpPKey, &x509Cert, NULL))
					{
						CMSError();
					}
					PKCS12_free(p12);
				}
			}
		}
		BIO_free(bioPKey);
	}

	if (NULL == evpPKey)
	{
		ZLog::Error("Can't Load P12 or PrivateKey File! Please Input The Correct File And Password!\n");
		return false;
	}

	if (NULL == x509Cert && !strSignerCertFile.empty())
	{
		BIO *bioCert = BIO_new_file(strSignerCertFile.c_str(), "r");
		if (NULL != bioCert)
		{
			x509Cert = PEM_read_bio_X509(bioCert, NULL, 0, NULL);
			if (NULL == x509Cert)
			{
				BIO_reset(bioCert);
				x509Cert = d2i_X509_bio(bioCert, NULL);
			}
			BIO_free(bioCert);
		}
	}

	if (NULL != x509Cert)
	{
		if (!X509_check_private_key(x509Cert, evpPKey))
		{
			X509_free(x509Cert);
			x509Cert = NULL;
		}
	}

	if (NULL == x509Cert)
	{
		for (size_t i = 0; i < jvProv["DeveloperCertificates"].size(); i++)
		{
			string strCertData = jvProv["DeveloperCertificates"][i].asData();
			BIO *bioCert = BIO_new_mem_buf(strCertData.c_str(), (int)strCertData.size());
			if (NULL != bioCert)
			{
				x509Cert = d2i_X509_bio(bioCert, NULL);
				if (NULL != x509Cert)
				{
					if (X509_check_private_key(x509Cert, evpPKey))
					{
						break;
					}
					X509_free(x509Cert);
					x509Cert = NULL;
				}
				BIO_free(bioCert);
			}
		}
	}

	if (NULL == x509Cert)
	{
		ZLog::Error("Can't Find Paired Certificate And PrivateKey!\n");
		return false;
	}

	if (!GetCertSubjectCN(x509Cert, m_strSubjectCN))
	{
		ZLog::Error("Can't Find Paired Certificate Subject Common Name!\n");
		return false;
	}

	m_evpPKey = evpPKey;
	m_x509Cert = x509Cert;
	return true;
}

bool ZSignAsset::GenerateCMS(const string &strCDHashData, const string &strCDHashesPlist, const string &strCodeDirectorySlotSHA1, const string &strAltnateCodeDirectorySlot256, string &strCMSOutput)
{
	return ::_GenerateCMS((X509 *)m_x509Cert, (EVP_PKEY *)m_evpPKey, strCDHashData, strCDHashesPlist, strCodeDirectorySlotSHA1, strAltnateCodeDirectorySlot256, strCMSOutput);
}
