find_program(ZSIGN_EXECUTABLE zsign PATHS /usr/local/bin)
message("zsign found at: ${ZSIGN_EXECUTABLE}")

add_test(Exec_cmd
    ${ZSIGN_EXECUTABLE} -v)

add_test(Encrypted_Cert
    ${ZSIGN_EXECUTABLE} -k Test/Encrypted.p12
            -m Test/test.mobileprovision
            -p 1234 
            Test/test.ipa
            -o /tmp/successful_Test.ipa)

add_test(Unencrypted_Cert
    ${ZSIGN_EXECUTABLE} -k Test/Unencrypted.p12 
            -m Test/test.mobileprovision 
            Test/test.ipa 
            -o /tmp/successful_Test.ipa)
