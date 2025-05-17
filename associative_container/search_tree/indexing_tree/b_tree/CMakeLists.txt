add_subdirectory(tests)

add_library(
        mp_os_assctv_cntnr_srch_tr_indxng_tr_b_tr
        include/b_tree.h
        src/hhh.cpp)

target_include_directories(
        mp_os_assctv_cntnr_srch_tr_indxng_tr_b_tr
        PUBLIC
        ./include
        "D:/ClionProjects/FIIT_FA_SP_IK/vcpkg/installed/x64-windows/include")

target_link_libraries(
        mp_os_assctv_cntnr_srch_tr_indxng_tr_b_tr
        PUBLIC
        mp_os_lggr_clnt_lggr)
target_link_libraries(
        mp_os_assctv_cntnr_srch_tr_indxng_tr_b_tr
        PUBLIC
        mp_os_assctv_cntnr_srch_tr)
target_link_libraries(
        mp_os_assctv_cntnr_srch_tr_indxng_tr_b_tr
        PUBLIC
        ${BOOST_LIBRARIES})


target_include_directories(
        mp_os_assctv_cntnr_srch_tr_indxng_tr_b_tr
        PUBLIC
        ${Boost_INCLUDE_DIRS})
target_link_directories(
        mp_os_assctv_cntnr_srch_tr_indxng_tr_b_tr
        PUBLIC
        ${Boost_LIBRARY_DIR}
)