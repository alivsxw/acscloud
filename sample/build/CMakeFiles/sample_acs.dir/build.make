# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.14

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /home/cmake-14-install/bin/cmake

# The command to remove a file.
RM = /home/cmake-14-install/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /mnt/hgfs/lianyong/check/svn/acsv2/sample

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /mnt/hgfs/lianyong/check/svn/acsv2/sample/build

# Include any dependencies generated for this target.
include CMakeFiles/sample_acs.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/sample_acs.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/sample_acs.dir/flags.make

CMakeFiles/sample_acs.dir/sample.c.o: CMakeFiles/sample_acs.dir/flags.make
CMakeFiles/sample_acs.dir/sample.c.o: ../sample.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/mnt/hgfs/lianyong/check/svn/acsv2/sample/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/sample_acs.dir/sample.c.o"
	/opt/arm-ca9-linux-uclibcgnueabihf-8.4.01/usr/bin/arm-linux-gcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/sample_acs.dir/sample.c.o   -c /mnt/hgfs/lianyong/check/svn/acsv2/sample/sample.c

CMakeFiles/sample_acs.dir/sample.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/sample_acs.dir/sample.c.i"
	/opt/arm-ca9-linux-uclibcgnueabihf-8.4.01/usr/bin/arm-linux-gcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /mnt/hgfs/lianyong/check/svn/acsv2/sample/sample.c > CMakeFiles/sample_acs.dir/sample.c.i

CMakeFiles/sample_acs.dir/sample.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/sample_acs.dir/sample.c.s"
	/opt/arm-ca9-linux-uclibcgnueabihf-8.4.01/usr/bin/arm-linux-gcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /mnt/hgfs/lianyong/check/svn/acsv2/sample/sample.c -o CMakeFiles/sample_acs.dir/sample.c.s

# Object files for target sample_acs
sample_acs_OBJECTS = \
"CMakeFiles/sample_acs.dir/sample.c.o"

# External object files for target sample_acs
sample_acs_EXTERNAL_OBJECTS =

sample_acs: CMakeFiles/sample_acs.dir/sample.c.o
sample_acs: CMakeFiles/sample_acs.dir/build.make
sample_acs: CMakeFiles/sample_acs.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/mnt/hgfs/lianyong/check/svn/acsv2/sample/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable sample_acs"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/sample_acs.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/sample_acs.dir/build: sample_acs

.PHONY : CMakeFiles/sample_acs.dir/build

CMakeFiles/sample_acs.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/sample_acs.dir/cmake_clean.cmake
.PHONY : CMakeFiles/sample_acs.dir/clean

CMakeFiles/sample_acs.dir/depend:
	cd /mnt/hgfs/lianyong/check/svn/acsv2/sample/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /mnt/hgfs/lianyong/check/svn/acsv2/sample /mnt/hgfs/lianyong/check/svn/acsv2/sample /mnt/hgfs/lianyong/check/svn/acsv2/sample/build /mnt/hgfs/lianyong/check/svn/acsv2/sample/build /mnt/hgfs/lianyong/check/svn/acsv2/sample/build/CMakeFiles/sample_acs.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/sample_acs.dir/depend
