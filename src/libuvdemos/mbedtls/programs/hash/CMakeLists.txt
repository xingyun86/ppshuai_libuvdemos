add_executable(hello hello.c)
target_link_libraries(hello mbedtls_static)

add_executable(generic_sum generic_sum.c)
target_link_libraries(generic_sum mbedtls_static)

install(TARGETS hello generic_sum
        DESTINATION "bin"
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
