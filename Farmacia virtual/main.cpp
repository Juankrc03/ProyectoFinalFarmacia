#include <iostream>
#include <fstream>
#include <limits>
#include <string>
#include <vector>
#include <algorithm>
#include <iomanip>
#include "json.hpp"   // Asegúrate: Propiedades C/C++ → General → $(ProjectDir)include

using nlohmann::json;

constexpr int REABASTECER_A = 40; // ← meta fija: si queda en 0, vuelve a 40

// ================= Utilidades =================
int readInt(const std::string& msg) {
    int v;
    while (true) {
        std::cout << msg;
        if (std::cin >> v) return v;
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Valor inválido.\n";
    }
}
std::string readStr(const std::string& msg) {
    std::cout << msg;
    std::string s; std::getline(std::cin >> std::ws, s);
    return s;
}
// ================= Archivos ===================
const std::string PRODUCTS_DB = "data/products.json";
const std::string CART_DB = "data/carrito.json";

json loadArr(const std::string& path) {
    std::ifstream f(path);
    if (!f.is_open()) return json::array();
    json j; f >> j;
    return j.is_array() ? j : json::array();
}
void saveArr(const std::string& path, const json& j) {
    std::ofstream f(path);
    f << j.dump(2);
}

json loadProducts() { return loadArr(PRODUCTS_DB); }
json loadCart() { return loadArr(CART_DB); }
void saveProducts(const json& j) { saveArr(PRODUCTS_DB, j); }
void saveCart(const json& j) { saveArr(CART_DB, j); }

// ============== Helpers de productos/carrito ==========
int findProd(const json& prods, int id) {
    for (size_t i = 0;i < prods.size();++i)
        if ((int)prods[i].value("id", -1) == id) return (int)i;
    return -1;
}
int findInCart(const json& cart, int pid) {
    for (size_t i = 0;i < cart.size();++i)
        if ((int)cart[i].value("productId", -1) == pid) return (int)i;
    return -1;
}

// ================== Vistas (solo lectura) =============
void verProductos(const json& prods) {
    std::cout << "\n=== LISTA DE PRODUCTOS ===\n";
    for (const auto& p : prods) {
        int stock = (int)p.value("stock", 0);
        std::string estado = (stock > 0) ? std::to_string(stock) : "Agotado";
        std::cout << "ID:" << (int)p.value("id", 0)
            << " | " << (std::string)p.value("nombre", "")
            << " | ₡" << std::fixed << std::setprecision(2) << (double)p.value("precio", 0.0)
            << " | stock:" << estado
            << " | cat:" << (std::string)p.value("categoria", "")
            << "\n";
    }
}
void verInventario(const json& prods) {
    std::cout << "\n=== INVENTARIO (Actual / Meta 40) ===\n";
    for (const auto& p : prods) {
        int s = (int)p.value("stock", 0);
        std::cout << "ID:" << (int)p.value("id", 0)
            << " | " << (std::string)p.value("nombre", "")
            << " | " << s << " / " << REABASTECER_A << "\n";
    }
}
void verCarrito(const json& cart, const json& prods) {
    std::cout << "\n=== CARRITO ===\n";
    if (cart.empty()) { std::cout << "(vacío)\n"; return; }
    double total = 0.0;
    for (const auto& it : cart) {
        int pid = (int)it.value("productId", -1);
        int cant = (int)it.value("cantidad", 0);
        double sub = (double)it.value("subTotal", 0.0);
        int idx = findProd(prods, pid);
        std::string nombre = idx >= 0 ? (std::string)prods[idx].value("nombre", "") : "(desconocido)";
        std::cout << "ID:" << pid << " | " << nombre
            << " | cant:" << cant
            << " | subTotal ₡" << std::fixed << std::setprecision(2) << sub << "\n";
        total += sub;
    }
    std::cout << "TOTAL: ₡" << std::fixed << std::setprecision(2) << total << "\n";
}


