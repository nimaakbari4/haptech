# Makefile created with the assistance of GPT-4.1

CHAI3D_DIR = external/chai3d-Franka-integration
CHAI3D_BUILD_DIR = $(CHAI3D_DIR)/build
GLFW_BUILD_DIR = $(CHAI3D_BUILD_DIR)/extras/GLFW
FRANKA_BUILD_DIR = $(CHAI3D_BUILD_DIR)/external/libfranka/src

SRC_DIR = src
BIN_DIR = bin
HDR_DIR = $(SRC_DIR)
SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
PROTO_SRCS = $(SRC_DIR)/franka_trial.pb.cc $(SRC_DIR)/franka_trial.grpc.pb.cc
PROTO_OBJS = $(BIN_DIR)/franka_trial.pb.o $(BIN_DIR)/franka_trial.grpc.pb.o
OBJECTS = $(patsubst $(SRC_DIR)/%.cpp, $(BIN_DIR)/%.o, $(SOURCES)) $(PROTO_OBJS)
TARGET = $(BIN_DIR)/core

CXX = g++
CXXFLAGS = -I$(CHAI3D_DIR)/src \
	-I/opt/homebrew/include/eigen3 \
	-I$(CHAI3D_DIR)/external/Eigen \
	-I$(CHAI3D_DIR)/external/glew/include \
	-I$(CHAI3D_DIR)/external/DHD/include \
	-I$(CHAI3D_DIR)/extras/GLFW/include \
	-I$(SRC_DIR) \
	-I/usr/include \
	-I/usr/local/include \
	-DLINUX -Wno-deprecated -std=c++17 -m64 -fsigned-char -O3

LDFLAGS = -L$(CHAI3D_BUILD_DIR) \
	-L$(GLFW_BUILD_DIR) \
	-L$(FRANKA_BUILD_DIR) \
	-L/usr/local/lib

LDLIBS = $(CHAI3D_BUILD_DIR)/libchai3d.a \
	$(GLFW_BUILD_DIR)/libglfw.a \
	-lgrpc++ -lgrpc -lprotobuf \
	-labsl_synchronization -labsl_symbolize -labsl_stacktrace -labsl_debugging_internal -labsl_demangle_internal -labsl_base -labsl_spinlock_wait \
	-lpthread -lrt -ldl -lGL -lGLU -lusb-1.0 \
	-lX11 -lXcursor -lXrandr -lXinerama

# Platform tweaks (Homebrew and macOS frameworks)
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
# Homebrew default prefix on Apple Silicon
 CXXFLAGS += -I/opt/homebrew/include -I/opt/homebrew/include/eigen3 -I/opt/homebrew/include/GL
 LDFLAGS += -L/opt/homebrew/lib -L/opt/homebrew/lib/glu
 # Drop Linux/OpenGL/X11-specific libs on macOS; frameworks are linked via chai3d
 LDLIBS := $(filter-out -lrt -lGL -lGLU -lX11 -lXcursor -lXrandr -lXinerama,$(LDLIBS))
 # Link GLU explicitly from Homebrew
 LDLIBS += -lGLU \
 	-framework Cocoa -framework IOKit -framework CoreVideo \
 	-framework CoreFoundation -framework CoreServices \
 	-framework CoreAudio -framework AudioToolbox -framework AudioUnit \
 	-framework OpenGL
 # Remove LINUX define
 CXXFLAGS := $(filter-out -DLINUX,$(CXXFLAGS))
 # Ensure grpc/protobuf libs from pkg-config are included
 LDLIBS += $(shell pkg-config --libs grpc++) $(shell pkg-config --libs protobuf)
endif

.PHONY: all cmake_chai3d_build clean

all: cmake_chai3d_build $(TARGET)

cmake_chai3d_build:
	@mkdir -p $(CHAI3D_BUILD_DIR)
	@cd $(CHAI3D_BUILD_DIR) && cmake -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -D CMAKE_CXX_FLAGS="-DTCP_KEEPIDLE=TCP_KEEPALIVE" .. && $(MAKE)


# Compile regular source files
$(BIN_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile protobuf/gRPC generated files
$(BIN_DIR)/franka_trial.pb.o: $(SRC_DIR)/franka_trial.pb.cc $(SRC_DIR)/franka_trial.pb.h | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BIN_DIR)/franka_trial.grpc.pb.o: $(SRC_DIR)/franka_trial.grpc.pb.cc $(SRC_DIR)/franka_trial.grpc.pb.h | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BIN_DIR):
	@mkdir -p $(BIN_DIR)

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) $(LDFLAGS) $(LDLIBS) -o $(TARGET)

clean:
	rm -rf $(BIN_DIR)/*.o $(TARGET)
