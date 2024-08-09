find_program(zsign_exec zsign PATHS Test)
find_file(encrypted_p12 NAMES Encrypted.p12 PATHS Test)
find_file(unencrypted_p12 NAMES Unencrypted.p12 PATHS Test)
find_file(provision_profile NAMES test.mobileprovision PATHS Test)
find_file(ipa NAMES test.ipa PATHS Test)

message("zsign found: ${zsign_exec}")
message("Encrypted .p12 found: ${encrypted_p12}")
message("Unencrypted .p12 found: ${unencrypted_p12}")
message("Provision profile found: ${provision_profile}")
message("iPA found: ${ipa}")


add_test(Exec_cmd
    ${zsign_exec} -v)

add_test(Encrypted_Cert
    ${zsign_exec} -k ${encrypted_p12}
            -m ${provision_profile}
            -p 1234 
            ${ipa}
            -o /tmp/successful_Test.ipa)

add_test(Unencrypted_Cert
    ${zsign_exec} -k ${unencrypted_p12}
            -m ${provision_profile}
            ${ipa} 
            -o /tmp/successful_Test.ipa)
