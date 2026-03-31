# ==========================================
# Configuración del Compilador
# ==========================================
CXX      := g++
CXXFLAGS := -std=c++23 -march=native -Iinclude
LDFLAGS  :=

# ==========================================
# Directorios
# ==========================================
SRC_DIR  := src
INC_DIR  := include
OBJ_DIR  := obj
BIN_DIR  := bin

# Nombre del ejecutable final
TARGET   := $(BIN_DIR)/arcana

# ==========================================
# Búsqueda automática de archivos
# ==========================================
# Encuentra todos los archivos .cpp dentro de src/ y sus subcarpetas
SRCS := $(shell find $(SRC_DIR) -name '*.cpp')

# Genera la lista de archivos objeto (.o) reemplazando la ruta src/ por obj/ y .cpp por .o
OBJS := $(SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

# ==========================================
# Perfiles de Compilación (Targets)
# ==========================================
all: deploy

# Perfil Debug: Máxima información de depuración, cero optimización
debug: CXXFLAGS += -g3 -Og -D_GLIBCXX_DEBUG -D_GLIBCXX_ASSERTIONS -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=3 -fstack-protector-all -fstack-clash-protection -fsanitize=undefined -fsanitize=leak -fsanitize=float-divide-by-zero -fsanitize=float-cast-overflow -fno-omit-frame-pointer -fPIE -pie -Wl,-z,now -Wl,-z,relro -Wl,-z,noexecstack -fno-common -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion -Wformat=2 -Wformat-overflow=2 -Wformat-truncation=2 -Wnull-dereference -Wduplicated-cond -Wduplicated-branches -Wlogical-op -Wuseless-cast -Wold-style-cast -Wdouble-promotion -Wtrampolines -Wfloat-equal -Wsuggest-attribute=noreturn -Wsuggest-final-types -Wsuggest-final-methods -Wplacement-new=2 -Wstrict-null-sentinel -Wmisleading-indentation -Wunused-macros -Wnon-virtual-dtor -Wimplicit-fallthrough -Wcast-qual -Wwrite-strings -Woverloaded-virtual -Wsuggest-override -Warray-bounds=2 -Wstrict-overflow=5 -Wstack-usage=8192 -Werror
debug: $(TARGET)

# Perfil Fast: Optimizaciones agresivas para velocidad de ejecución
fast: CXXFLAGS += -O0 -pipe
fast: $(TARGET)

# Perfil Deploy: Optimizado, sin aserciones (NDEBUG) y "strippeado" (sin símbolos para que pese menos)
deploy: CXXFLAGS += -O3 -fno-rtti -fomit-frame-pointer
deploy: $(TARGET)

# Perfil Shy: Optimizado para tamaño
shy: CXXFLAGS += -Oz -s -fno-asynchronous-unwind-tables -fno-plt -fno-rtti -ffunction-sections -fdata-sections -fno-stack-protector -fno-ident -fvisibility=hidden -fvisibility-inlines-hidden -Wl,--gc-sections -Wl,--build-id=none
shy: $(TARGET)
	@echo "🗜️ Aplicando post-procesado agresivo en $(TARGET)..."
	@strip --strip-all --remove-section=.comment --remove-section=.note $(TARGET)
	@sstrip $(TARGET) || echo "⚠️ sstrip no encontrado, saltando..."
	@upx --ultra-brute $(TARGET) || echo "⚠️ upx no encontrado, saltando..."

# ==========================================
# Reglas principales
# ==========================================
# La regla por defecto al ejecutar solo 'make'
all: $(TARGET)

# Regla para enlazar (linkear) el ejecutable final
$(TARGET): $(OBJS)
	@echo "Enlazando $@"
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

# Regla para compilar cada .cpp en un .o
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@echo "Compilando $<"
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) -c $< -o $@

# Regla para limpiar los binarios y objetos (make clean)
clean:
	@echo "Limpiando archivos de compilación..."
	@rm -rf $(OBJ_DIR) $(BIN_DIR)

# Evita conflictos con archivos que se llamen "all" o "clean"
.PHONY: all clean
