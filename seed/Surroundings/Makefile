# Project Name
TARGET = Surroundings

# Sources
CPP_SOURCES = Surroundings.cpp \
              modules/samplevoice/samplevoice.cpp

# Library Locations
LIBDAISY_DIR = ../../libDaisy
DAISYSP_DIR = ../../DaisySP

# Includes FatFS source files within project.
USE_FATFS = 1

# # Linker flags
# # This is not really required, used only for profiling! Increases executable size by ~8kB
# LDFLAGS = -u _printf_float

# Core location, and generic makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile