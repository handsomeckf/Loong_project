# 包含头文件路径
include_directories(${PROJECT_SOURCE_DIR}/HMAC/Inc)

add_library(utils_hmac ./scr/utils_hmac.c)
add_library(utils_md5 ./scr/utils_md5.c)
add_library(utils_sha1 ./scr/utils_sha1.c)

target_link_libraries(utils_hmac PRIVATE utils_md5 PRIVATE utils_sha1)
