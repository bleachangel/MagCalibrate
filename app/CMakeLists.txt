# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

SET(SRC_LIST src/main/jni/bilinear_form.c
             src/main/jni/bilinear_pairing.c
             src/main/jni/choleski.c
             src/main/jni/copy_matrix.c
             src/main/jni/copy_vector.c
             src/main/jni/gauss_aux_elimination.c
             src/main/jni/gauss_elimination.c
             src/main/jni/get_submatrix.c
             src/main/jni/hessenberg_elementary.c
             src/main/jni/identity_matrix.c
             src/main/jni/interchange_cols.c
             src/main/jni/interchange_rows.c
             src/main/jni/lower_triangular.c
             src/main/jni/matrix_transposed_x_a_matrix.c
             src/main/jni/matrix_transposed_x_the_matrix.c
             src/main/jni/matrix_x_a_matrix_transposed.c
             src/main/jni/matrix_x_its_transpose.c
             src/main/jni/multiply_matrices.c
             src/main/jni/multiply_matrix_by_vector.c
             src/main/jni/multiply_vector_by_matrix.c
             src/main/jni/qr_hessenberg_matrix.c
             src/main/jni/set_submatrix.c
             src/main/jni/transpose_square_matrix.c
             src/main/jni/upper_triangular.c
             src/main/jni/zero_matrix.c
             src/main/jni/MadMagCalibrate.cpp
             src/main/jni/Calibration.cpp
)

add_library( # Sets the name of the library.
             jni_MadMagCalibrate

             # Sets the library as a shared library.
             SHARED

             # Provides a relative path to your source file(s).
             ${SRC_LIST})

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
              log-lib

              # Specifies the name of the NDK library that
              # you want CMake to locate.
              log )

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
                       jni_MadMagCalibrate

                       # Links the target library to the log library
                       # included in the NDK.
                       ${log-lib} )