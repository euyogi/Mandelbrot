# --- Configuração ---
CXX = g++
CXXFLAGS = -std=c++17 -Isrc -Wall -Wextra -O3
LDFLAGS = -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi
SHADERC = glslc
EXECUTABLE = Vulkan

# --- Diretórios ---
SRC_DIR = src
BUILD_DIR = build
SHADER_DIR = shaders

# --- Descoberta Automática de Arquivos C++ ---
SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS = $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SOURCES))

# --- Lista Explícita de Shaders ---
SHADERS_SPV = $(SHADER_DIR)/vert.spv $(SHADER_DIR)/frag.spv

# --- Alvos Principais ---
.PHONY: all run clean shaders

# O alvo 'all' depende do executável. Ao chamar 'make', este será o alvo padrão.
all: $(EXECUTABLE)

# Regra para criar o executável final.
# Depende de todos os arquivos objeto e dos shaders compilados.
$(EXECUTABLE): $(OBJECTS) $(SHADERS_SPV)
	@echo "==> Linking $(EXECUTABLE)..."
	$(CXX) $(OBJECTS) -o $@ $(LDFLAGS)

# Regra para compilar arquivos .cpp de 'src/' para arquivos .o em 'build/'.
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(BUILD_DIR)
	@echo "==> Compiling $<..."
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Regra para compilar o shader de vértice.
$(SHADER_DIR)/vert.spv: $(SHADER_DIR)/shader.vert
	@mkdir -p $(SHADER_DIR)
	@echo "==> Compiling shader: $<"
	$(SHADERC) $< -o $@

# Regra para compilar o shader de fragmento.
$(SHADER_DIR)/frag.spv: $(SHADER_DIR)/shader.frag
	@mkdir -p $(SHADER_DIR)
	@echo "==> Compiling shader: $<"
	$(SHADERC) $< -o $@

# Alvo para forçar a compilação de todos os shaders.
shaders: $(SHADERS_SPV)

# --- Alvos Utilitários ---
run: all
	./$(EXECUTABLE)

clean:
	@echo "==> Cleaning up..."
	rm -f $(EXECUTABLE)
	rm -rf $(BUILD_DIR)
	rm -f $(SHADER_DIR)/*.spv

