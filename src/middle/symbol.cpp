// symbol.cpp

#include "Symbol.hpp"

#include "Includes.hpp"

/* --- Dt --- */
bool Dt::operator==(const Dt& otro) const {
  // Si ambos son nulos, son iguales
  if (!this->valor && !otro.valor) { return true ; }
  // Si uno es uno y el otro no, son distintos
  if (!this->valor || !otro.valor) { return false; }

  return this->valor->esIgual(otro.valor.get());
}

bool Dt::operator<(const Dt& otro) const {
  return valor < otro.valor;

}

bool Dt::esPrimitivo() const { //...
  switch(valor->kind) {
    case TypeKind::VOID   :
    case TypeKind::BOOLEAN:
    case TypeKind::CHAR   :
    case TypeKind::INTEGER:
    case TypeKind::FLOAT  :
    case TypeKind::STRING :
    case TypeKind::ARRAY  : {
      return true;
    }

    default: {
      return false;
    }

  }
}

std::string Dt::tipoString() const {
  if (valor == nullptr) { return "???"; }
  return (es_const? "const": "") + valor->toString();
}

/* --- Gestor de Tablas Maestro --- */
GestorTablas::GestorTablas() {
  root = std::make_unique<Scope>(nullptr);
  scopeActual = root.get();

}

bool GestorTablas::getLectura() const { return lectura; }
void GestorTablas::switchMode() { lectura = !lectura; }

void GestorTablas::resetScope() {
  lectura = true;
  root->resetNavegacion();
  scopeActual = root.get();

}

void GestorTablas::entrarScope() {
  std::cout << "[61, symbol.cpp] entrarScope\n";
  if (!lectura) {
    auto nuevo_hijo = std::make_unique<Scope>(scopeActual);
    Scope* ptr_hijo = nuevo_hijo.get();
    scopeActual->hijos.push_back(std::move(nuevo_hijo));
    scopeActual = ptr_hijo;

  } else {
    if (scopeActual->hijo_actual < scopeActual->hijos.size()) {
      scopeActual = scopeActual->hijos[scopeActual->hijo_actual++].get();
    }
  }
}

Scope* GestorTablas::getScopeActual() { return scopeActual; }

void GestorTablas::setScopeActual(Scope* scope) { scopeActual = scope; }

void GestorTablas::salirScope() {
  std::cout << "[79, symbol.cpp] salirScope\n";
  if (scopeActual->padre) {
    scopeActual = scopeActual->padre;

  } else {
    std::cerr << "Error: Intento de salir de un scope raíz o nulo.\n";

  }
}

void GestorTablas::promoverScopes(size_t cantidad) {
  if (cantidad == 0 || root->hijos.size() <= cantidad) { return ; }

  auto inicio_corte = root->hijos.end() - cantidad;

  std::vector<std::unique_ptr<Scope>> temp_scopes;
  for (auto it = inicio_corte; it != root->hijos.end(); ++it) {
    temp_scopes.push_back(std::move(*it));
  }

  root->hijos.erase(inicio_corte, root->hijos.end());

  root->hijos.insert(
    root->hijos.begin(),
    std::make_move_iterator(temp_scopes.begin()),
    std::make_move_iterator(temp_scopes.end())
  );
}

InfoVariable* GestorTablas::buscarVariable(const std::string& name) {
  Scope* cursor = scopeActual;
  while (cursor != nullptr) {
    auto it = cursor->variables.find(name);
    if (it != cursor->variables.end()) {
      return &(it->second);
    }
    cursor = cursor->padre;
  }
  return nullptr;

}

bool GestorTablas::añadirVariable(const std::string& name, InfoVariable info) {

  if (scopeActual->variables.find(name) != scopeActual->variables.end()) {
    return false;

  }

  scopeActual->variables[name] = std::move(info);
  return true;

}

bool GestorTablas::añadirFuncion(const std::string& name, InfoFuncion info) {
  std::cout << "[116, symbol.cpp] Añadiendo: '" << name << "' en: " << scopeActual << '\n';

  auto& overloads = scopeActual->funciones[name];

  for (const auto& func : overloads) {
    if (func.firma == info.firma) {
      return false;
    }
  }

  overloads.push_back(std::move(info));

  return true;

}

std::vector<InfoFuncion>* GestorTablas::buscarFuncionName(const std::string& name) {
  std::cout << "[152, symbol.cpp] Buscando: '" << name << "' en: " << scopeActual << '\n';
  Scope* cursor = scopeActual;

  while (cursor != nullptr) {
    auto it = cursor->funciones.find(name);

    if (it != cursor->funciones.end()) {
      return &(it->second);

    }

    cursor = cursor->padre;
    std::cout << cursor << '\n';

  }

  std::cout << '\'' << name << "' no encontrada\n";
  return nullptr;

}

InfoFuncion* GestorTablas::buscarFuncionFirma(const std::string& name, const std::string& firma) {
  std::vector<InfoFuncion>* candidatas = buscarFuncionName(name);

  if (candidatas) {
    for (auto& func : *candidatas) {
      if (func.firma == firma) {
        return &func;
      }
    }
  }

  return nullptr;

}

InfoFuncion* GestorTablas::getCurrentFunction() {
  std::cout << "[170, symbol.cpp] getCurrentFunction\n";

  if (!pilaFuncs.empty()) {
    std::cout << "[173, symbol.cpp] pilaFuncs.back()\n";
    return pilaFuncs.back();

  }

  std::cout << "[178, symbol.cpp] pilaFuncs empty\n";

  return nullptr;

}

void GestorTablas::pushFunction(InfoFuncion* function) { pilaFuncs.push_back(function);   }

void GestorTablas::popFunction() {
  if (!pilaFuncs.empty()) { pilaFuncs.pop_back(); }

}

bool GestorTablas::añadirStruct(const std::string& name, InfoStruct info) {
  if (root->structs.count(name)) { return false; }

  root->structs[name] = std::move(info);
  return true;

}

bool GestorTablas::actualizarStruct(const std::string& name, InfoStruct infoActualizada) {
  InfoStruct* info = buscarStruct(name);

  if (info != nullptr) {
    *info = std::move(infoActualizada);
    return true;
  }

  return false;

}

InfoStruct* GestorTablas::buscarStruct(const std::string& name) {

  auto it = root->structs.find(name);
  if (it != root->structs.end()) { return &(it->second); }

  return nullptr;

}

void GestorTablas::entrarScopeTemplate() {
  pilaTempaltes.push_back(std::unordered_map<std::string, InfoTemplateParam>());

}

void GestorTablas::salirScopeTemplate() {
  if (!pilaTempaltes.empty()) {
    pilaTempaltes.pop_back();

  } else {
    std::cerr << "Error: Interno de salir de un scope de templates root\n";

  }
}

bool GestorTablas::añadirTemplate(const std::string& name, InfoTemplate info) {
  if (root->templates.count(name)) { return false; }

  root->templates[name] = std::move(info);
  return true;

}

bool GestorTablas::updateTemplate(const std::string& name, Scope* scope) {
  InfoTemplate* tmpl = buscarTemplate(name);

  if (tmpl) {
    tmpl->scope_def = scope;
    return true;
  }

  return false;

}

InfoTemplate* GestorTablas::buscarTemplate(const std::string& name) {
  auto it = root->templates.find(name);
  if (it != root->templates.end()) { return &(it->second); }

  return nullptr;

}

bool GestorTablas::añadirTemplateParam(const std::string& name, InfoTemplateParam info) {
  if (pilaTempaltes.empty()) { return false; }

  auto& contextoActual = pilaTempaltes.back();
  if (contextoActual.count(name)) {
    return false;
  }

  contextoActual[name] = std::move(info);
  return true;

}

InfoTemplateParam* GestorTablas::buscarTemplateParam(const std::string& name) {
  for (auto it = pilaTempaltes.rbegin(); it != pilaTempaltes.rend(); ++it) {
    auto found = it->find(name);
    if (found != it->end()) {
      return &(found->second);

    }

  }

  return nullptr;

}
