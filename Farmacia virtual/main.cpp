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
