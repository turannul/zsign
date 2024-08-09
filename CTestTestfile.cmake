add_test(Exec_cmd
    COMMAND zsign -v)

add_test(Encrypted_Cert
    COMMAND zsign -k Test/Encrypted.p12
            -m Test/test.mobileprovision
            -p 1234 
            Test/test.ipa
            -o /tmp/successful_Test.ipa)

add_test(Unencrypted_Cert
    COMMAND zsign -k Test/Unencrypted.p12 
            -m Test/test.mobileprovision 
            Test/test.ipa 
            -o /tmp/successful_Test.ipa)
