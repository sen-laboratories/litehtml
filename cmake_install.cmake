# Install script for directory: /boot/home/Develop/html-renderer/litehtml

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/boot/system")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set path to fallback-tool for dependency-resolution.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/bin/objdump")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "libraries" OR NOT CMAKE_INSTALL_COMPONENT)
  foreach(file
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/liblitehtml.so.0.9.0"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/liblitehtml.so.0"
      )
    if(EXISTS "${file}" AND
       NOT IS_SYMLINK "${file}")
      file(RPATH_CHECK
           FILE "${file}"
           RPATH "")
    endif()
  endforeach()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES
    "/boot/home/Develop/html-renderer/litehtml/liblitehtml.so.0.9.0"
    "/boot/home/Develop/html-renderer/litehtml/liblitehtml.so.0"
    )
  foreach(file
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/liblitehtml.so.0.9.0"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/liblitehtml.so.0"
      )
    if(EXISTS "${file}" AND
       NOT IS_SYMLINK "${file}")
      if(CMAKE_INSTALL_DO_STRIP)
        execute_process(COMMAND "/bin/strip" "${file}")
      endif()
    endif()
  endforeach()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "libraries" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES "/boot/home/Develop/html-renderer/litehtml/liblitehtml.so")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/litehtml" TYPE FILE FILES
    "/boot/home/Develop/html-renderer/litehtml/include/litehtml.h"
    "/boot/home/Develop/html-renderer/litehtml/include/litehtml/background.h"
    "/boot/home/Develop/html-renderer/litehtml/include/litehtml/borders.h"
    "/boot/home/Develop/html-renderer/litehtml/include/litehtml/codepoint.h"
    "/boot/home/Develop/html-renderer/litehtml/include/litehtml/css_length.h"
    "/boot/home/Develop/html-renderer/litehtml/include/litehtml/css_margins.h"
    "/boot/home/Develop/html-renderer/litehtml/include/litehtml/css_offsets.h"
    "/boot/home/Develop/html-renderer/litehtml/include/litehtml/css_position.h"
    "/boot/home/Develop/html-renderer/litehtml/include/litehtml/css_selector.h"
    "/boot/home/Develop/html-renderer/litehtml/include/litehtml/css_parser.h"
    "/boot/home/Develop/html-renderer/litehtml/include/litehtml/css_tokenizer.h"
    "/boot/home/Develop/html-renderer/litehtml/include/litehtml/document.h"
    "/boot/home/Develop/html-renderer/litehtml/include/litehtml/document_container.h"
    "/boot/home/Develop/html-renderer/litehtml/include/litehtml/el_anchor.h"
    "/boot/home/Develop/html-renderer/litehtml/include/litehtml/el_base.h"
    "/boot/home/Develop/html-renderer/litehtml/include/litehtml/el_before_after.h"
    "/boot/home/Develop/html-renderer/litehtml/include/litehtml/el_body.h"
    "/boot/home/Develop/html-renderer/litehtml/include/litehtml/el_break.h"
    "/boot/home/Develop/html-renderer/litehtml/include/litehtml/el_cdata.h"
    "/boot/home/Develop/html-renderer/litehtml/include/litehtml/el_comment.h"
    "/boot/home/Develop/html-renderer/litehtml/include/litehtml/el_div.h"
    "/boot/home/Develop/html-renderer/litehtml/include/litehtml/el_font.h"
    "/boot/home/Develop/html-renderer/litehtml/include/litehtml/el_image.h"
    "/boot/home/Develop/html-renderer/litehtml/include/litehtml/el_link.h"
    "/boot/home/Develop/html-renderer/litehtml/include/litehtml/el_para.h"
    "/boot/home/Develop/html-renderer/litehtml/include/litehtml/el_script.h"
    "/boot/home/Develop/html-renderer/litehtml/include/litehtml/el_space.h"
    "/boot/home/Develop/html-renderer/litehtml/include/litehtml/el_style.h"
    "/boot/home/Develop/html-renderer/litehtml/include/litehtml/el_table.h"
    "/boot/home/Develop/html-renderer/litehtml/include/litehtml/el_td.h"
    "/boot/home/Develop/html-renderer/litehtml/include/litehtml/el_text.h"
    "/boot/home/Develop/html-renderer/litehtml/include/litehtml/el_title.h"
    "/boot/home/Develop/html-renderer/litehtml/include/litehtml/el_tr.h"
    "/boot/home/Develop/html-renderer/litehtml/include/litehtml/element.h"
    "/boot/home/Develop/html-renderer/litehtml/include/litehtml/encodings.h"
    "/boot/home/Develop/html-renderer/litehtml/include/litehtml/html.h"
    "/boot/home/Develop/html-renderer/litehtml/include/litehtml/html_tag.h"
    "/boot/home/Develop/html-renderer/litehtml/include/litehtml/html_microsyntaxes.h"
    "/boot/home/Develop/html-renderer/litehtml/include/litehtml/iterators.h"
    "/boot/home/Develop/html-renderer/litehtml/include/litehtml/media_query.h"
    "/boot/home/Develop/html-renderer/litehtml/include/litehtml/os_types.h"
    "/boot/home/Develop/html-renderer/litehtml/include/litehtml/style.h"
    "/boot/home/Develop/html-renderer/litehtml/include/litehtml/stylesheet.h"
    "/boot/home/Develop/html-renderer/litehtml/include/litehtml/table.h"
    "/boot/home/Develop/html-renderer/litehtml/include/litehtml/tstring_view.h"
    "/boot/home/Develop/html-renderer/litehtml/include/litehtml/types.h"
    "/boot/home/Develop/html-renderer/litehtml/include/litehtml/url.h"
    "/boot/home/Develop/html-renderer/litehtml/include/litehtml/url_path.h"
    "/boot/home/Develop/html-renderer/litehtml/include/litehtml/utf8_strings.h"
    "/boot/home/Develop/html-renderer/litehtml/include/litehtml/web_color.h"
    "/boot/home/Develop/html-renderer/litehtml/include/litehtml/num_cvt.h"
    "/boot/home/Develop/html-renderer/litehtml/include/litehtml/css_properties.h"
    "/boot/home/Develop/html-renderer/litehtml/include/litehtml/line_box.h"
    "/boot/home/Develop/html-renderer/litehtml/include/litehtml/render_item.h"
    "/boot/home/Develop/html-renderer/litehtml/include/litehtml/render_flex.h"
    "/boot/home/Develop/html-renderer/litehtml/include/litehtml/render_image.h"
    "/boot/home/Develop/html-renderer/litehtml/include/litehtml/render_inline.h"
    "/boot/home/Develop/html-renderer/litehtml/include/litehtml/render_table.h"
    "/boot/home/Develop/html-renderer/litehtml/include/litehtml/render_inline_context.h"
    "/boot/home/Develop/html-renderer/litehtml/include/litehtml/render_block_context.h"
    "/boot/home/Develop/html-renderer/litehtml/include/litehtml/render_block.h"
    "/boot/home/Develop/html-renderer/litehtml/include/litehtml/master_css.h"
    "/boot/home/Develop/html-renderer/litehtml/include/litehtml/string_id.h"
    "/boot/home/Develop/html-renderer/litehtml/include/litehtml/formatting_context.h"
    "/boot/home/Develop/html-renderer/litehtml/include/litehtml/flex_item.h"
    "/boot/home/Develop/html-renderer/litehtml/include/litehtml/flex_line.h"
    "/boot/home/Develop/html-renderer/litehtml/include/litehtml/gradient.h"
    "/boot/home/Develop/html-renderer/litehtml/containers/haiku/container_haiku.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/litehtml" TYPE FILE FILES "/boot/home/Develop/html-renderer/litehtml/cmake/litehtmlConfig.cmake")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/litehtml/litehtmlTargets.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/litehtml/litehtmlTargets.cmake"
         "/boot/home/Develop/html-renderer/litehtml/CMakeFiles/Export/1858d3296707c77b4f85418fd0121701/litehtmlTargets.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/litehtml/litehtmlTargets-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/litehtml/litehtmlTargets.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/litehtml" TYPE FILE FILES "/boot/home/Develop/html-renderer/litehtml/CMakeFiles/Export/1858d3296707c77b4f85418fd0121701/litehtmlTargets.cmake")
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^()$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/litehtml" TYPE FILE FILES "/boot/home/Develop/html-renderer/litehtml/CMakeFiles/Export/1858d3296707c77b4f85418fd0121701/litehtmlTargets-noconfig.cmake")
  endif()
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/boot/home/Develop/html-renderer/litehtml/src/gumbo/cmake_install.cmake")

endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
if(CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "/boot/home/Develop/html-renderer/litehtml/install_local_manifest.txt"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
if(CMAKE_INSTALL_COMPONENT)
  if(CMAKE_INSTALL_COMPONENT MATCHES "^[a-zA-Z0-9_.+-]+$")
    set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
  else()
    string(MD5 CMAKE_INST_COMP_HASH "${CMAKE_INSTALL_COMPONENT}")
    set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INST_COMP_HASH}.txt")
    unset(CMAKE_INST_COMP_HASH)
  endif()
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "/boot/home/Develop/html-renderer/litehtml/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
