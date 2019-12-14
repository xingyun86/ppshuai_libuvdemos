add_executable(dh_client dh_client.c)
target_link_libraries(dh_client mbedtls_static)

add_executable(dh_genprime dh_genprime.c)
target_link_libraries(dh_genprime mbedtls_static)

add_executable(dh_server dh_server.c)
target_link_libraries(dh_server mbedtls_static)

add_executable(ecdh_curve25519 ecdh_curve25519.c)
target_link_libraries(ecdh_curve25519 mbedtls_static)

add_executable(ecdsa ecdsa.c)
target_link_libraries(ecdsa mbedtls_static)

add_executable(gen_key gen_key.c)
target_link_libraries(gen_key mbedtls_static)

add_executable(key_app key_app.c)
target_link_libraries(key_app mbedtls_static)

add_executable(key_app_writer key_app_writer.c)
target_link_libraries(key_app_writer mbedtls_static)

add_executable(mpi_demo mpi_demo.c)
target_link_libraries(mpi_demo mbedtls_static)

add_executable(rsa_genkey rsa_genkey.c)
target_link_libraries(rsa_genkey mbedtls_static)

add_executable(rsa_sign rsa_sign.c)
target_link_libraries(rsa_sign mbedtls_static)

add_executable(rsa_verify rsa_verify.c)
target_link_libraries(rsa_verify mbedtls_static)

add_executable(rsa_sign_pss rsa_sign_pss.c)
target_link_libraries(rsa_sign_pss mbedtls_static)

add_executable(rsa_verify_pss rsa_verify_pss.c)
target_link_libraries(rsa_verify_pss mbedtls_static)

add_executable(rsa_encrypt rsa_encrypt.c)
target_link_libraries(rsa_encrypt mbedtls_static)

add_executable(rsa_decrypt rsa_decrypt.c)
target_link_libraries(rsa_decrypt mbedtls_static)

add_executable(pk_sign pk_sign.c)
target_link_libraries(pk_sign mbedtls_static)

add_executable(pk_verify pk_verify.c)
target_link_libraries(pk_verify mbedtls_static)

add_executable(pk_encrypt pk_encrypt.c)
target_link_libraries(pk_encrypt mbedtls_static)

add_executable(pk_decrypt pk_decrypt.c)
target_link_libraries(pk_decrypt mbedtls_static)

install(TARGETS dh_client dh_genprime dh_server key_app mpi_demo rsa_genkey rsa_sign rsa_verify rsa_encrypt rsa_decrypt pk_encrypt pk_decrypt pk_sign pk_verify gen_key
        DESTINATION "bin"
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
