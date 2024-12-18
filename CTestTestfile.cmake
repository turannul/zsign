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
    ${zsign_exec} -V)

add_test(Exec_cmd_longArg
    ${zsign_exec} --version)

add_test(Encrypted_Cert
    ${zsign_exec} -p ${encrypted_p12}
            -m ${provision_profile}
            -P 1234 
            ${ipa}
            -o /tmp/Encrypted_Test.ipa)

add_test(Encrypted_Cert_longArg
    ${zsign_exec} --pkey ${encrypted_p12}
            --prov ${provision_profile}
            --password 1234 
            ${ipa}
            --output /tmp/Encrypted_Test.ipa)

add_test(Unencrypted_Cert
    ${zsign_exec} -p ${unencrypted_p12}
            -m ${provision_profile}
            ${ipa} 
            -o /tmp/Unencrypted_Test.ipa)

add_test(Unencrypted_Cert_longArg
    ${zsign_exec} --pkey ${unencrypted_p12}
            --prov ${provision_profile}
            ${ipa} 
            --output /tmp/Unencrypted_Test.ipa)

add_test(Bundle_ID
    ${zsign_exec} -p ${unencrypted_p12}
            -m ${provision_profile}
            ${ipa} 
            -o /tmp/Unencrypted_Test.ipa
            -b xyz.turannul.test)

add_test(Bundle_ID_longArg
    ${zsign_exec} --pkey ${unencrypted_p12}
            --prov ${provision_profile}
            ${ipa} 
            --output /tmp/Unencrypted_Test.ipa
            --bundle_id xyz.turannul.test)

add_test(Bundle_Name
    ${zsign_exec} -p ${unencrypted_p12}
            -m ${provision_profile}
            ${ipa} 
            -o /tmp/Unencrypted_Test.ipa
            -n TestApp)

add_test(Bundle_Name_longArg
    ${zsign_exec} --pkey ${unencrypted_p12}
            --prov ${provision_profile}
            ${ipa} 
            --output /tmp/Unencrypted_Test.ipa
            --bundle_name TestApp)

add_test(Bundle_Version
    ${zsign_exec} -p ${unencrypted_p12}
            -m ${provision_profile}
            ${ipa} 
            -o /tmp/Unencrypted_Test.ipa
            -v 1.0.0)

add_test(Bundle_Version_longArg
    ${zsign_exec} --pkey ${unencrypted_p12}
            --prov ${provision_profile}
            ${ipa} 
            --output /tmp/Unencrypted_Test.ipa
            --bundle_version 1.0.0)

add_test(Remove_Embedded
    ${zsign_exec} -p ${unencrypted_p12}
            -m ${provision_profile}
            ${ipa} 
            -o /tmp/Unencrypted_Test_RemoveEmbedded.ipa
            -e)

add_test(Remove_Embedded_longArg
    ${zsign_exec} --pkey ${unencrypted_p12}
            --prov ${provision_profile}
            ${ipa} 
            --output /tmp/Unencrypted_Test_RemoveEmbedded.ipa
            --remove_embedded)

add_test(Remove_WatchBundle
    ${zsign_exec} -p ${unencrypted_p12}
            -m ${provision_profile}
            ${ipa} 
            -o /tmp/Unencrypted_Test_RemoveWatch.ipa
            -r)

add_test(Remove_WatchBundle_longArg
    ${zsign_exec} --pkey ${unencrypted_p12}
            --prov ${provision_profile}
            ${ipa} 
            --output /tmp/Unencrypted_Test_RemoveWatch.ipa
            --remove_watch)