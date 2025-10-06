# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "D:/Users/esp/v5.4.2/esp-idf/components/bootloader/subproject")
  file(MAKE_DIRECTORY "D:/Users/esp/v5.4.2/esp-idf/components/bootloader/subproject")
endif()
file(MAKE_DIRECTORY
  "F:/3.5TFT/3.5tft_4spi/3.5tf_4spi_lvgl/build/bootloader"
  "F:/3.5TFT/3.5tft_4spi/3.5tf_4spi_lvgl/build/bootloader-prefix"
  "F:/3.5TFT/3.5tft_4spi/3.5tf_4spi_lvgl/build/bootloader-prefix/tmp"
  "F:/3.5TFT/3.5tft_4spi/3.5tf_4spi_lvgl/build/bootloader-prefix/src/bootloader-stamp"
  "F:/3.5TFT/3.5tft_4spi/3.5tf_4spi_lvgl/build/bootloader-prefix/src"
  "F:/3.5TFT/3.5tft_4spi/3.5tf_4spi_lvgl/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "F:/3.5TFT/3.5tft_4spi/3.5tf_4spi_lvgl/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "F:/3.5TFT/3.5tft_4spi/3.5tf_4spi_lvgl/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
