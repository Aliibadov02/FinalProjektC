#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <fstream>

using namespace std;

void logToFile(const string& message) {
    ofstream file("log.txt", ios::app);
    if (file.is_open()) {
        file << message << endl;
        file.close();
    }
}


bool RegisterUser(string& outUsername, string& outRole) {
    string username, password, role;
    cout << "Enter Username: ";
    cin >> username;
    cout << "Enter password: ";
    cin >> password;
    cout << "Rol (admin/user): ";
    cin >> role;

    if (role != "admin" && role != "user") {
        cout << "Invalid role entered. (admin/user)" << endl;
        return false;
    }

    ifstream infile("users.txt");
    string u, p, r;
    while (infile >> u >> p >> r) {
        if (u == username) {
            cout << "This username already exists" << endl;
            return false;
        }
    }
    infile.close();

    ofstream outfile("users.txt", ios::app);
    outfile << username << " " << password << " " << role << endl;
    outUsername = username;
    outRole = role;
    logToFile("New registration: " + username + " (" + role + ")");
    cout << "Registration is succesfull!" << endl;
    return true;
}

bool loginUser(string& outUsername, string& outRole) {
    string username, password;
    cout << "Username: ";
    cin >> username;
    cout << "Password: ";
    cin >> password;

    ifstream infile("users.txt");
    string u, p, r;
    while (infile >> u >> p >> r) {
        if (u == username && p == password) {
            outUsername = username;
            outRole = r;
            logToFile("Logged in: " + username + " (" + r + ")");
            cout << "Logged is succesfull" << endl;
            return true;
        }
    }

    cout << "Username or password is incorrect!" << endl;
    return false;
}


class Ingredient {
public:
    string name;
    double quantity;
    double pricePerKilo;

    Ingredient(string n = "", double q = 0, double p = 0)
        : name(n), quantity(q), pricePerKilo(p) {
    }
};

class Dish {
public:
    string name;
    map<string, double> ingredients;

    Dish(string n = "") : name(n) {}

    void addIngredient(const string& ingName, double amount) {
        ingredients[ingName] = amount;
    }
};

class Inventory {
private:
    map<string, Ingredient> stock;

public:
    void addIngredient(const Ingredient& ing) {
        stock[ing.name].name = ing.name;
        stock[ing.name].quantity += ing.quantity;
        stock[ing.name].pricePerKilo = ing.pricePerKilo;
    }

    bool AmountofIngredients(const map<string, double>& required) {
        for (auto& pair : required) {
            if (stock[pair.first].quantity < pair.second)
                return false;
        }
        return true;
    }

    bool useIngredients(const map<string, double>& used) {
        if (!AmountofIngredients(used)) return false;
        for (auto& pair : used) {
            stock[pair.first].quantity -= pair.second;
        }
        return true;
    }

    double CalculateCost(const map<string, double>& used) {
        double total = 0;
        for (auto& pair : used) {
            total += pair.second * stock[pair.first].pricePerKilo;
        }
        return total;
    }

    void ShowStock() const {
        cout << "-- Warehouse --" << endl;
        for (auto& item : stock) {
            cout << item.second.name << ": " << item.second.quantity << " kg " << endl;
        }
    }

    bool isIngredientAvailable(const string& ingName) {
        return stock.count(ingName) && stock[ingName].quantity > 0;
    }
};

class Admin {
private:
    Inventory& inventory;
    vector<Dish> menu;
    double budget = 1000;
    double income = 0;
    double expense = 0;

public:
    Admin(Inventory& inv) : inventory(inv) {}

    void addDish(const Dish& dish) {
        menu.push_back(dish);
        logToFile("Admin added dish: " + dish.name);
    }

    void addIngredientToInventory(const Ingredient& ing) {
        double cost = ing.quantity * ing.pricePerKilo;
        if (cost > budget) {
            cout << "There are not enough funds in the budget!" << endl;
            return;
        }
        budget -= cost;
        expense += cost;
        inventory.addIngredient(ing);
        cout << ing.name << " Added succesfull (" << ing.quantity << " kg)" << endl;
        logToFile("Admin added ingredient: " + ing.name + ", " + to_string(ing.quantity) + " kg, " + to_string(ing.pricePerKilo) + " AZN/kg");
    }

    void receivePayment(double amount) {
        income += amount;
        budget += amount;
        logToFile("Admin received payment: " + to_string(amount) + " AZN ");
    }

    void showStatistics() const {
        cout << "\n-- Statistics --" << endl;
        cout << "Budget: " << budget << " AZN\n";
        cout << "It comes: " << income << " AZN\n";
        cout << "Cost: " << expense << " AZN\n";
        logToFile("Admin viewed statistics");
    }

    vector<Dish> getMenu() const {
        return menu;
    }
};

class Pannier {
public:
    Dish dish;
    map<string, double> customIngredients;

    Pannier(Dish d) : dish(d) {}

    void addExtraIngredient(const string& name, double amount) {
        customIngredients[name] += amount;
    }

    map<string, double> getTotalIngredients() const {
        map<string, double> total = dish.ingredients;
        for (auto& pair : customIngredients) {
            total[pair.first] += pair.second;
        }
        return total;
    }
};

class Cart {
public:
    vector<Pannier> items;

    void addItem(const Dish& dish) {
        items.emplace_back(dish);
    }

    void clearCart() {
        items.clear();
    }
};

class User {
public:
    string username;
    Cart cart;
    vector<string> orderHistory;

    User(string name) : username(name) {}

    void viewDishInfo(const Dish& dish) {
        cout << "Food: " << dish.name << "\nComposition:" << endl;
        for (auto& ing : dish.ingredients) {
            cout << " - " << ing.first << ": " << ing.second << " kg" << endl;
        }
    }

    void PlaceOrder(Inventory& inv, Admin& admin) {
        double totalCost = 0;

        for (auto& item : cart.items) {
            map<string, double> ingredients = item.getTotalIngredients();
            if (!inv.AmountofIngredients(ingredients)) {
                cout << "There are not enough ingredients for the order!" << endl;
                logToFile("User " + username + " Attempted order but ingredients were insufficient.");
                return;
            }
        }

        for (auto& item : cart.items) {
            map<string, double> ingredients = item.getTotalIngredients();
            inv.useIngredients(ingredients);
            totalCost += inv.CalculateCost(ingredients);
        }

        double payment = totalCost * 1.5;
        admin.receivePayment(payment);
        orderHistory.push_back("Order - sum: " + to_string(payment));
        cart.clearCart();
        cout << "Order completed successfully! Payment: " << payment << " AZN" << endl;
        logToFile("User " + username + " Placed order. Paid: " + to_string(payment) + " AZN");
    }

    void ShowOrderHistory() {
        cout << "--- " << username << " ucun tarixce ---\n";
        for (auto& entry : orderHistory) {
            cout << entry << endl;
        }
        logToFile("User " + username + " viewed order history");
    }
};

int main() {
    string currentUser, currentRole;

    while (true) {
        cout << "\t\t \n- Wellcome our Restaurant -" << endl;
        cout << "\t\t 1. Registration" << endl;
        cout << "\t\t 2. Sign in" << endl;
        cout << "\t\t 0. Close app" << endl;
        cout << "Choose one: ";
        int chooice;
        cin >> chooice;

        if (chooice == 1) {
            if (RegisterUser(currentUser, currentRole)) break;
        }
        else if (chooice == 2) {
            if (loginUser(currentUser, currentRole)) break;
        }
        else if (chooice == 0) {
            logToFile("App closed!");
            return 0;
        }
        else {
            cout << "Wrong choice. Please try again." << endl;
        }
    }

    Inventory inv;
    Admin admin(inv);
    User user(currentUser);

    Dish pizza("Pizza");
    pizza.addIngredient("Pomidor", 1);
    pizza.addIngredient("Pendir", 0.05);
    admin.addDish(pizza);


    if (currentRole == "admin") {
        int choice;
        while (true) {
            cout << "\n-- ADMİN PAGE --" << endl;
            cout << "1. Add ingredient" << endl;
            cout << "2. Add food" << endl;
            cout << "3. See statistics" << endl;
            cout << "4. Warehouse information" << endl;
            cout << "0. Log out" << endl;
            cout << "Choice: ";
            cin >> choice;

            switch (choice) {
            case 1: {
                string name;
                double qty, price;
                cout << "Name of Ingrident: "; cin >> name;
                cout << "Quantity (kg): "; cin >> qty;
                cout << "Price of 1 kg: "; cin >> price;
                admin.addIngredientToInventory(Ingredient(name, qty, price));
                break;
            }
            case 2: {
                string name, ingName;
                double qty;
                Dish newDish;
                cout << "Name of food: "; cin >> name;
                newDish.name = name;
                cout << "Add the ingredients (0 for stop):" << endl;
                while (true) {
                    cout << "Name of ingrident: "; cin >> ingName;
                    if (ingName == "0") break;
                    cout << "Quantity (kg): "; cin >> qty;
                    newDish.addIngredient(ingName, qty);
                }
                admin.addDish(newDish);
                break;
            }
            case 3:
                admin.showStatistics();
                break;
            case 4:
                inv.ShowStock();
                break;
            case 0:
                return 0;
            default:
                cout << "Wrong choice.\n";
            }
        }
    }

    else if (currentRole == "user") {
        int choice;
        while (true) {
            cout << "\n-- USER PAGE --" << endl;
            cout << "1. See the menu" << endl;
            cout << "2. Add food to cart" << endl;
            cout << "3. Add additional ingredients" << endl;
            cout << "4. Complete the order" << endl;
            cout << "5. See history" << endl;
            cout << "6. Warehouse information" << endl;
            cout << "0. Log out" << endl;
            cout << "Choice one: ";
            cin >> choice;

            switch (choice) {
            case 1: {
                auto menu = admin.getMenu();
                for (size_t i = 0; i < menu.size(); i++) {
                    cout << i + 1 << ". " << menu[i].name << endl;
                    user.viewDishInfo(menu[i]);
                }
                logToFile("User " + user.username + " viewed menu.");
                break;
            }
            case 2: {
                auto menu = admin.getMenu();
                int index;
                cout << "Food you want to add: " << endl;
                cin >> index;
                if (index >= 1 && index <= menu.size()) {
                    user.cart.addItem(menu[index - 1]);
                    cout << "The meal was added immediately." << endl;
                    logToFile("User " + user.username + " added dish to cart: " + menu[index - 1].name);
                }
                else {
                    cout << "Wrong choise." << endl;
                }
                break;
            }
            case 3: {
                string name;
                double qty;
                if (user.cart.items.empty()) {
                    cout << "The basket is empty!" << endl;
                    break;
                }
                cout << "The ingredient you want to add is simple: ";
                cin >> name;
                cout << "Quantity (kg): ";
                cin >> qty;
                user.cart.items.back().addExtraIngredient(name, qty);
                cout << "Added." << endl;
                logToFile("User " + user.username + " Added extra ingredient to last dish: " + name + ", " + to_string(qty) + " kg");
                break;
            }
            case 4:
                user.PlaceOrder(inv, admin);
                break;
            case 5:
                user.ShowOrderHistory();
                break;
            case 6:
                inv.ShowStock();
                logToFile("Inventory viewed by user.");
                break;
            case 0:
                return 0;
            default:
                cout << "Wrong choice." << endl;
            }
        }
    }

    return 0;
}