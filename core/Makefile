# Makefile created with the assistance of GPT-4.1

CHAI3D_DIR = external/chai3d-Franka-integration
CHAI3D_BUILD_DIR = $(CHAI3D_DIR)/build
GLFW_BUILD_DIR = $(CHAI3D_BUILD_DIR)/extras/GLFW
FRANKA_BUILD_DIR = $(CHAI3D_BUILD_DIR)/external/libfranka/src
PROTO_DIR = ../proto
PROTO_FILE = $(PROTO_DIR)/franka_trial.proto

PROTO_DIR ?= ../proto
PROTO_FILE ?= $(PROTO_DIR)/franka_trial.proto

SRC_DIR = src
BIN_DIR = bin
HDR_DIR = $(SRC_DIR)
SOURCES = $(wildcard $(SRC_DIR)/*.cpp) $(wildcard $(SRC_DIR)/*/*.cpp)
PROTO_SRCS = $(SRC_DIR)/franka_trial.pb.cc $(SRC_DIR)/franka_trial.grpc.pb.cc
PROTO_OBJS = $(BIN_DIR)/franka_trial.pb.o $(BIN_DIR)/franka_trial.grpc.pb.o
# Create object file names by replacing src/ with bin/ and .cpp with .o
MAIN_OBJS = $(SOURCES:$(SRC_DIR)/%.cpp=$(BIN_DIR)/%.o)
OBJECTS = $(MAIN_OBJS) $(PROTO_OBJS)
TARGET = $(BIN_DIR)/core

CXX = g++
CXXFLAGS = -I$(CHAI3D_DIR)/src \
	-I$(CHAI3D_DIR)/external/Eigen \
	-I$(CHAI3D_DIR)/external/glew/include \
	-I$(CHAI3D_DIR)/external/DHD/include \
	-I$(CHAI3D_DIR)/extras/GLFW/include \
	-I$(CHAI3D_DIR)/external/libfranka/include \
	-I$(CHAI3D_DIR)/external/libfranka/common/include \
	-I$(SRC_DIR) \
	-I$(SRC_DIR)/proto \
	-I$(SRC_DIR)/objects \
	-I$(SRC_DIR)/trial_conditions \
	-I/usr/include \
	-I/usr/local/include \
	-I/usr/include/eigen3 \
	-DLINUX -Wno-deprecated -std=c++17 -m64 -fsigned-char

# Enable debug info if requested: make DEBUG=1
ifeq ($(DEBUG),1)
  CXXFLAGS := $(filter-out -O3,$(CXXFLAGS))
  CXXFLAGS += -O0 -g -fno-omit-frame-pointer -DDEBUG
endif

LDFLAGS = -L$(CHAI3D_BUILD_DIR) \
	-L$(GLFW_BUILD_DIR) \
	-L$(FRANKA_BUILD_DIR) \
	-L/usr/lib/x86_64-linux-gnu


LDLIBS = $(CHAI3D_BUILD_DIR)/libchai3d.a \
$(GLFW_BUILD_DIR)/libglfw.a \
$(CHAI3D_BUILD_DIR)/external/libfranka/libfranka.so.0.21.3 \
 -lgrpc++ -lgrpc -lgpr -lprotobuf \
 -lcares \
 -lz -lssl -lcrypto \
 -lpthread -lrt -ldl -lGL -lGLU -lusb-1.0 \
 -lX11 -lXcursor -lXrandr -lXinerama \
 -lPocoFoundation -lPocoNet -lPocoNetSSL -lPocoCrypto

# Platform tweaks (Homebrew and macOS frameworks)
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
# Homebrew default prefix on Apple Silicon
 CXXFLAGS += -I/opt/homebrew/include -I/opt/homebrew/include/eigen3 -I/opt/homebrew/include/GL
 # Re-enable deprecated std::binder1st/2nd expected by older Eigen in CHAI3D
 CXXFLAGS += -D_LIBCPP_ENABLE_CXX17_REMOVED_BINDERS
 LDFLAGS += -L/opt/homebrew/lib -L/opt/homebrew/lib/glu
 # Drop Linux/OpenGL/X11-specific libs on macOS; frameworks are linked via chai3d
 LDLIBS := $(filter-out -lrt -lGL -lGLU -lX11 -lXcursor -lXrandr -lXinerama,$(LDLIBS))
 # Drop Linux-specific libfranka .so on macOS
 LDLIBS := $(filter-out $(CHAI3D_BUILD_DIR)/external/libfranka/libfranka.so.0.21.3,$(LDLIBS))
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

.PHONY: all cmake_chai3d_build clean generate_proto proto_clean

all: generate_proto cmake_chai3d_build $(TARGET)

cmake_chai3d_build:
	@mkdir -p $(CHAI3D_BUILD_DIR)
	@cd $(CHAI3D_BUILD_DIR) && cmake -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
		-DCMAKE_CXX_FLAGS="-DTCP_KEEPIDLE=TCP_KEEPALIVE" \
		-DCMAKE_PREFIX_PATH="/usr" \
		-DPoco_DIR="/usr/lib/x86_64-linux-gnu/cmake/Poco" \
		.. && $(MAKE)


# Compile regular source files
$(BIN_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BIN_DIR)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile source files from subdirectories
$(BIN_DIR)/objects/%.o: $(SRC_DIR)/objects/%.cpp | $(BIN_DIR)
	@mkdir -p $(BIN_DIR)/objects
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BIN_DIR)/trial_conditions/%.o: $(SRC_DIR)/trial_conditions/%.cpp | $(BIN_DIR)
	@mkdir -p $(BIN_DIR)/trial_conditions
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile protobuf/gRPC generated files
$(BIN_DIR)/franka_trial.pb.o: $(SRC_DIR)/franka_trial.pb.cc $(SRC_DIR)/franka_trial.pb.h | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BIN_DIR)/franka_trial.grpc.pb.o: $(SRC_DIR)/franka_trial.grpc.pb.cc $(SRC_DIR)/franka_trial.grpc.pb.h | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BIN_DIR):
	@mkdir -p $(BIN_DIR)
	@mkdir -p $(BIN_DIR)/objects
	@mkdir -p $(BIN_DIR)/trial_conditions

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) $(LDFLAGS) $(LDLIBS) -o $(TARGET)

# --- Protobuf / gRPC codegen ---
generate_proto:
	@echo "[PROTO] Generating C++ sources from $(PROTO_FILE)"
	@mkdir -p $(SRC_DIR)
	@protoc -I$(PROTO_DIR) --cpp_out=$(SRC_DIR) --grpc_out=$(SRC_DIR) --plugin=protoc-gen-grpc=$$(which grpc_cpp_plugin) $(PROTO_FILE)

proto_clean:
	@rm -f $(SRC_DIR)/franka_trial.pb.cc $(SRC_DIR)/franka_trial.pb.h \
	       $(SRC_DIR)/franka_trial.grpc.pb.cc $(SRC_DIR)/franka_trial.grpc.pb.h

clean:
	rm -rf $(BIN_DIR)

LDLIBS += $(shell pkg-config --libs grpc++ protobuf)
