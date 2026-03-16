#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "ngtcp2::ngtcp2" for configuration "Release"
set_property(TARGET ngtcp2::ngtcp2 APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(ngtcp2::ngtcp2 PROPERTIES
  IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/lib/libngtcp2.dll.a"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/bin/libngtcp2-16.dll"
  )

list(APPEND _cmake_import_check_targets ngtcp2::ngtcp2 )
list(APPEND _cmake_import_check_files_for_ngtcp2::ngtcp2 "${_IMPORT_PREFIX}/lib/libngtcp2.dll.a" "${_IMPORT_PREFIX}/bin/libngtcp2-16.dll" )

# Import target "ngtcp2::ngtcp2_crypto_ossl" for configuration "Release"
set_property(TARGET ngtcp2::ngtcp2_crypto_ossl APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(ngtcp2::ngtcp2_crypto_ossl PROPERTIES
  IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/lib/libngtcp2_crypto_ossl.dll.a"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/bin/libngtcp2_crypto_ossl-0.dll"
  )

list(APPEND _cmake_import_check_targets ngtcp2::ngtcp2_crypto_ossl )
list(APPEND _cmake_import_check_files_for_ngtcp2::ngtcp2_crypto_ossl "${_IMPORT_PREFIX}/lib/libngtcp2_crypto_ossl.dll.a" "${_IMPORT_PREFIX}/bin/libngtcp2_crypto_ossl-0.dll" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
