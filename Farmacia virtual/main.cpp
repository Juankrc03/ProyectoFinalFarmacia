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


// ================== Acciones ==========================
void agregarAlCarrito(json& cart, const json& prods) {
    int pid = readInt("ID del producto a agregar: ");
    int idx = findProd(prods, pid);
    if (idx < 0) { std::cout << "No existe ese producto.\n"; return; }

    int stock = (int)prods[idx].value("stock", 0);
    double precio = (double)prods[idx].value("precio", 0.0);
    std::cout << "Producto: " << (std::string)prods[idx].value("nombre", "")
        << " | stock: " << stock << " | precio: ₡" << precio << "\n";
    int cant = readInt("Cantidad: ");
    if (cant <= 0 || cant > stock) { std::cout << "Cantidad inválida.\n"; return; }

    int cidx = findInCart(cart, pid);
    if (cidx >= 0) {
        int nueva = (int)cart[cidx].value("cantidad", 0) + cant;
        if (nueva > stock) { std::cout << "Excede el stock.\n"; return; }
        cart[cidx]["cantidad"] = nueva;
        cart[cidx]["subTotal"] = nueva * precio;
    }
    else {
        cart.push_back(json{ {"productId",pid},{"cantidad",cant},{"subTotal",cant * precio} });
    }
    saveCart(cart);
    std::cout << "Agregado.\n";
}

void quitarDelCarrito(json& cart) {
    int pid = readInt("ID del producto a quitar del carrito: ");
    int cidx = findInCart(cart, pid);
    if (cidx < 0) { std::cout << "Ese producto no está en el carrito.\n"; return; }
    cart.erase(cart.begin() + cidx);
    saveCart(cart);
    std::cout << "Quitado.\n";
}

void vaciarCarrito(json& cart) {
    cart = json::array();
    saveCart(cart);
    std::cout << "Carrito vaciado.\n";
}

// ======= Checkout con reabastecimiento a 40 ===========
void checkout(json& cart, json& prods) {
    if (cart.empty()) { std::cout << "Carrito vacío.\n"; return; }

    // Resumen
    std::cout << "\n=== CHECKOUT PAGE ===\n";
    verCarrito(cart, prods);

    std::string metodo = readStr("Método de pago (Efectivo/Tarjeta/SINPE): ");
    std::string conf = readStr("Confirmar compra? (s/n): ");
    if (conf != "s" && conf != "S") { std::cout << "Cancelado.\n"; return; }

    // Validación stock y rebaja
    for (auto& it : cart) {
        int pid = (int)it.value("productId", -1);
        int cant = (int)it.value("cantidad", 0);
        int idx = findProd(prods, pid);
        if (idx < 0) { std::cout << "Producto no existe.\n"; return; }
        int stock = (int)prods[idx].value("stock", 0);
        if (cant > stock) { std::cout << "Sin stock suficiente para ID:" << pid << "\n"; return; }
    }
    for (auto& it : cart) {
        int pid = (int)it.value("productId", -1);
        int cant = (int)it.value("cantidad", 0);
        int idx = findProd(prods, pid);
        prods[idx]["stock"] = (int)prods[idx].value("stock", 0) - cant;
        if ((int)prods[idx].value("stock", 0) <= 0) {
            prods[idx]["stock"] = REABASTECER_A; // reabastece a 40 si quedó en 0
        }
    }
    saveProducts(prods);

    // Vaciar carrito
    cart = json::array();
    saveCart(cart);

    std::cout << "Pago aceptado por " << metodo << ". ¡Gracias por su compra!\n";
}

// ======= Checkout con reabastecimiento a 40 ===========
void checkout(json& cart, json& prods) {
    if (cart.empty()) { std::cout << "Carrito vacío.\n"; return; }

    // Resumen
    std::cout << "\n=== CHECKOUT PAGE ===\n";
    verCarrito(cart, prods);

    std::string metodo = readStr("Método de pago (Efectivo/Tarjeta/SINPE): ");
    std::string conf = readStr("Confirmar compra? (s/n): ");
    if (conf != "s" && conf != "S") { std::cout << "Cancelado.\n"; return; }

    // Validación stock y rebaja
    for (auto& it : cart) {
        int pid = (int)it.value("productId", -1);
        int cant = (int)it.value("cantidad", 0);
        int idx = findProd(prods, pid);
        if (idx < 0) { std::cout << "Producto no existe.\n"; return; }
        int stock = (int)prods[idx].value("stock", 0);
        if (cant > stock) { std::cout << "Sin stock suficiente para ID:" << pid << "\n"; return; }
    }
    for (auto& it : cart) {
        int pid = (int)it.value("productId", -1);
        int cant = (int)it.value("cantidad", 0);
        int idx = findProd(prods, pid);
        prods[idx]["stock"] = (int)prods[idx].value("stock", 0) - cant;
        if ((int)prods[idx].value("stock", 0) <= 0) {
            prods[idx]["stock"] = REABASTECER_A; // reabastece a 40 si quedó en 0
        }
    }
    saveProducts(prods);

    // Vaciar carrito
    cart = json::array();
    saveCart(cart);

    std::cout << "Pago aceptado por " << metodo << ". ¡Gracias por su compra!\n";
}