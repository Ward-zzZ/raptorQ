# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

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
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/ward/raptorQ

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/ward/raptorQ/build

# Include any dependencies generated for this target.
include CMakeFiles/temp.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/temp.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/temp.dir/flags.make

CMakeFiles/temp.dir/RaptorQ.c.o: CMakeFiles/temp.dir/flags.make
CMakeFiles/temp.dir/RaptorQ.c.o: ../RaptorQ.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ward/raptorQ/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/temp.dir/RaptorQ.c.o"
	/bin/x86_64-linux-gnu-gcc-9 $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/temp.dir/RaptorQ.c.o   -c /home/ward/raptorQ/RaptorQ.c

CMakeFiles/temp.dir/RaptorQ.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/temp.dir/RaptorQ.c.i"
	/bin/x86_64-linux-gnu-gcc-9 $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/ward/raptorQ/RaptorQ.c > CMakeFiles/temp.dir/RaptorQ.c.i

CMakeFiles/temp.dir/RaptorQ.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/temp.dir/RaptorQ.c.s"
	/bin/x86_64-linux-gnu-gcc-9 $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/ward/raptorQ/RaptorQ.c -o CMakeFiles/temp.dir/RaptorQ.c.s

CMakeFiles/temp.dir/temp.c.o: CMakeFiles/temp.dir/flags.make
CMakeFiles/temp.dir/temp.c.o: ../temp.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ward/raptorQ/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object CMakeFiles/temp.dir/temp.c.o"
	/bin/x86_64-linux-gnu-gcc-9 $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/temp.dir/temp.c.o   -c /home/ward/raptorQ/temp.c

CMakeFiles/temp.dir/temp.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/temp.dir/temp.c.i"
	/bin/x86_64-linux-gnu-gcc-9 $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/ward/raptorQ/temp.c > CMakeFiles/temp.dir/temp.c.i

CMakeFiles/temp.dir/temp.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/temp.dir/temp.c.s"
	/bin/x86_64-linux-gnu-gcc-9 $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/ward/raptorQ/temp.c -o CMakeFiles/temp.dir/temp.c.s

# Object files for target temp
temp_OBJECTS = \
"CMakeFiles/temp.dir/RaptorQ.c.o" \
"CMakeFiles/temp.dir/temp.c.o"

# External object files for target temp
temp_EXTERNAL_OBJECTS =

temp: CMakeFiles/temp.dir/RaptorQ.c.o
temp: CMakeFiles/temp.dir/temp.c.o
temp: CMakeFiles/temp.dir/build.make
temp: CMakeFiles/temp.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/ward/raptorQ/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking C executable temp"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/temp.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/temp.dir/build: temp

.PHONY : CMakeFiles/temp.dir/build

CMakeFiles/temp.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/temp.dir/cmake_clean.cmake
.PHONY : CMakeFiles/temp.dir/clean

CMakeFiles/temp.dir/depend:
	cd /home/ward/raptorQ/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/ward/raptorQ /home/ward/raptorQ /home/ward/raptorQ/build /home/ward/raptorQ/build /home/ward/raptorQ/build/CMakeFiles/temp.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/temp.dir/depend

