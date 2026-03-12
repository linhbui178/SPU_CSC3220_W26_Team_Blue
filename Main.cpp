//
// Created by Noel Mehari on 2/28/26.
//
//
// Created by Noel Mehari on 2/25/26.
//
#include <mysqlx/xdevapi.h>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <string>
#include <string_view>
#include <cctype>
#include <limits>

#include "CurrentUser.h"
#include "Item.h"

using std::cout;
using std::endl;
using std::string;
using std::cin;
using std::cerr;
using std::isspace;
using std::numeric_limits;
using std::streamsize;
using std::max;

void PasswordValidation() {
    cout <<
        "Password must be between 10-50 characters\n"
        "Password must contain at least one number\n"
        "Password must contain at least one uppercase letter\n"
        "Password must contain at least one lowercase letter\n"
        "Password may not contain any Special characters\n"
        "Enter password here: ";
}

bool validDate(const std::string& input) {

    tm tm = {};
    std::istringstream ss(input);

    ss >> std::get_time(&tm, "%Y-%m-%d");

    if (ss.fail() || !ss.eof()) {
        return false;
    }

    tm.tm_hour = 0;
    tm.tm_min = 0;
    tm.tm_sec = 0;

    time_t parsedTime = mktime(&tm);
    if (parsedTime == -1) {
        return false;
    }

    std::tm *normalized = localtime(&parsedTime);

    return normalized->tm_year == tm.tm_year &&
           normalized->tm_mon == tm.tm_mon &&
           normalized->tm_mday == tm.tm_mday;
}

int findUserId(mysqlx::Table &Users, const string &Email) {
    auto result = Users.select("UserId")
    .where("Email = :email")
    .bind("email", Email)
    .execute();

    auto row = result.fetchOne();

    if (row) {
        return row[0];
    }
    return -1;
}

int findItemId(mysqlx::Table &Item, const string &ItemName, const int &UserId) {
    auto result = Item.select("ItemID")
    .where("ItemName = :item_name AND UserId = :user_id")
    .bind("item_name", ItemName)
    .bind("user_id", UserId)
    .execute();

    auto row = result.fetchOne();

    if (row) {
        return row[0];
    }
    return -1;
}

bool validateLength(const string &name) {
    if (name.length() > 254) {
        return false;
    }
    return true;
}

bool itemExists(const string &ItemName, const int &UserId) {
    mysqlx::Session session("127.0.0.1", 33060, "root", "noelmehari1");
    mysqlx::Schema DB = session.getSchema("PantryPal");
    mysqlx::Table Item = DB.getTable("Item");

    auto Read = Item
    .select("ItemName")
    .where("ItemName = :item_name and UserId = :user_id")
    .bind("item_name", ItemName)
    .bind("user_id", UserId)
    .execute();

    if (Read.count() > 0) {
        return true;
    }

    return false;
}

bool categoryExists(const int &CatId) {
    mysqlx::Session session("127.0.0.1", 33060, "root", "noelmehari1");
    mysqlx::Schema DB = session.getSchema("PantryPal");
    mysqlx::Table Category = DB.getTable("Category");

    auto Read = Category
    .select("CatId")
    .where("CatId = :cat_id")
    .bind("cat_id", CatId)
    .execute();

    if (Read.count() > 0) {
        return true;
    }

    return false;
}

int findCatId(const string &CatName) {
    mysqlx::Session session("127.0.0.1", 33060, "root", "noelmehari1");
    mysqlx::Schema DB = session.getSchema("PantryPal");
    mysqlx::Table Category = DB.getTable("Category");

    auto result = Category.select("CatId")
    .where("CatName = :cat_name")
    .bind("cat_name", CatName)
    .execute();

    auto row = result.fetchOne();

    if (row) {
        return row[0];
    }
    return -1;
}

void printCategories() {
    try {
        mysqlx::Session session("127.0.0.1", 33060, "root", "noelmehari1");
        session.sql("USE PantryPal").execute();

        auto result = session.sql("SELECT CatId, CatName "
                                        "FROM Category "
                                        "ORDER BY CatId")
                                        .execute();

        cout << "\n=== Categories ===\n";

        auto row = result.fetchOne();

        if (!row) {
            cout << "\nNo categories available" << endl;
        }

        else {
            do {
                cout << row[0].get<int>() << "   " << row[1].get<string>() << endl;
            }
            while (row = result.fetchOne());
        }
    }
    catch (const mysqlx::Error &err) {
        cerr << "\nConnection error: " << err.what() << endl;
    }
}

void viewItems(const int &UserId) {
    int option;

    try {
        mysqlx::Session session("127.0.0.1", 33060, "root", "noelmehari1");
        session.sql("USE PantryPal").execute();

        auto result = session.sql("SELECT ItemName, CatName, ItemQuant, ItemDesc, "
                                        "DATE_FORMAT(Item.Added_on, '%Y-%m-%d %H:%i:%s'), "
                                        "DATE_FORMAT(Item.Exp_date, '%Y-%m-%d %H:%i:%s') "
                                        "FROM Item "
                                        "JOIN Category ON Item.CatId = Category.CatId "
                                        //"JOIN Expiration ON Item.ItemId = Expiration.ItemId "   // waiting for Robel to finish Expiration, ViewItems wont run otherwise
                                        "WHERE Item.UserId = ?")
                                        .bind(UserId)
                                        .execute();

        cout << "\n=== Your Items ===\n";

        auto row = result.fetchOne();

        if (!row) {
            cout << "\nYou have no items" << endl;
        }

        else {
            do {
                cout << "Item Name: " << row[0].get<string>();
                cout << "\nCategory: " << row[1].get<string>() << endl;
                cout << "\nItem Quantity: " << row[2].get<int>();
                if (!row[3].isNull()) {
                cout << "\nItem Description: " << row[3].get<string>();
                }
                cout << "\nItem Added on: " << row[4].get<string>();
                cout << "\nItem Expires on: " << row[5].get<string>();
            }
            while (row == result.fetchOne());
        }
    }
    catch (const mysqlx::Error &err) {
        cerr << "\nConnection error: " << err.what() << endl;
    }

    while (true) {
        cout << "\n\n1. Back to main menu" << endl;
        cout << "Enter your choice: ";

        if (!(cin >> option)) {
            cout << "Invalid input!" << endl;
            cout << "Please enter a number" << endl;
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }

        if (option == 1) {
            break;
        }

        cout << "Invalid choice" << endl;
    }
}

void addItem(const int &UserId) {
    int ItemQuant, CatId, option;
    string ItemName, ItemDesc, ItemCategory, CatName, ItemExp, Added_on;

    while (true) {
        cout << "\n=== Add Item ===\n";

        // get item name

        while (true) {
            cout << "\nEnter item name (Maximum characters is 254): ";
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            getline(cin, ItemName);
            if (!validateLength(ItemName)) {
                cout << "Item name too long" << endl;
            }
            else if (itemExists(ItemName, UserId)) {
                cout << "Item already exists" << endl;
            }
            else {
                break;
            }
        }

        // get item quantity
        while (true) {
            cout << "Enter item quantity: ";
            if (!(cin >> ItemQuant)) {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cout << "Invalid input!" << endl;
                cout << "Please enter a number" << endl;
                continue;
            }
            break;
        }

        // get item category
        printCategories();
        while (true) {
            cout << "Enter category number: ";
            if (!(cin >> CatId)) {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cout << "Invalid input!" << endl;
                cout << "Please enter a number" << endl;
                continue;
            }
            if (!categoryExists(CatId)) {
                cout << "Invalid choice" << endl;
            }
            break;
        }

        // get item description

        while (true) {
            cout << "Enter item description (Press Enter to skip): ";
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            getline(cin, ItemDesc);
            if (ItemDesc.empty()) {
                cout << "Description skipped" << endl;
                break;
            }
            if (validateLength(ItemDesc)) {
                break;
            }
            cout << "Item description too long" << endl;
        }


        while (true) {

            cout << "Input the item's Expiration date (YYYY-MM-DD): ";
            getline(cin, ItemExp);

            if (ItemExp.empty()) {
                cout << "Expiration date required\n";
                continue;
            }

            if (!validDate(ItemExp)) {
                cout << "Invalid date format\n";
                continue;
            }

            tm expTm = {};
            std::istringstream expStream(ItemExp);
            expStream >> std::get_time(&expTm, "%Y-%m-%d");

            expTm.tm_hour = 0;
            expTm.tm_min = 0;
            expTm.tm_sec = 0;

            time_t expTime = mktime(&expTm);

            time_t now = time(nullptr);
            tm todayTm = *localtime(&now);
            todayTm.tm_hour = 0;
            todayTm.tm_min = 0;
            todayTm.tm_sec = 0;

            time_t todayTime = mktime(&todayTm);

            if (expTime < todayTime) {
                cout << "Expiration date cannot be before today\n";
                continue;
            }

            cout << "Valid expiration date entered.\n";
            break;
        }


        // add item
        try {
            mysqlx::Session session("127.0.0.1", 33060, "root", "noelmehari1");
            mysqlx::Schema DB = session.getSchema("PantryPal");
            mysqlx::Table Item = DB.getTable("Item");

            Item.insert("UserId", "CatId", "ItemName", "ItemQuant", "ItemDesc", "Exp_date")
                .values(UserId, CatId, ItemName, ItemQuant, ItemDesc, ItemExp)
                .execute();

            cout << "\nItem added successfully!" << endl;
        }
        catch (const mysqlx::Error &err) {
            cerr << "\nConnection error: " << err.what() << endl;
        }

        while (true) {
            cout << "\n1. Add another item" << endl;
            cout << "2. Back to main menu" << endl;
            cout << "Enter your choice: ";

            if (!(cin >> option)) {
                cout << "Invalid input!" << endl;
                cout << "Please enter a number" << endl;
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                continue;
            }

            if (option != 1 && option != 2) {
                cout << "Invalid choice" << endl;
                continue;
            }

            break;
        }

        if (option == 1) {
            continue;
        }
        if (option == 2) {
            break;
        }
    }
}

void updateItem(const int &UserId) {

    int choice, ItemId, ItemQuant, newItemQuant, option, CatId;
    string ItemName, ItemCategory, ItemDesc, newItemName, newItemCat, newItemDesc;

    while (true) {

        cout << "\n=== Update Item ===\n";

        try {
            mysqlx::Session session("127.0.0.1", 33060, "root", "noelmehari1");
            mysqlx::Schema DB = session.getSchema("PantryPal");
            mysqlx::Table Item = DB.getTable("Item");

            auto result = Item.select("COUNT(*)")
                .where("UserID = :user_id")
                .bind("user_id", UserId)
                .execute();
            auto row = result.fetchOne();
            int count = row[0];

            if (count == 0) {
                cout << "\nYou have no items to update" << endl;
                cout << "Heading back to main menu" << endl;
                break;
            }
        }
        catch (const mysqlx::Error &err) {
            cerr << "\nConnection error: " << err.what() << endl;
        }

        // get item name
        while (true) {
            cout << "Enter item name: ";
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            getline(cin, ItemName);
            if (itemExists(ItemName, UserId)) {
                break;
            }
            cout << "\nItem does not exist" << endl;
        }

        // print item details TO BE IMPLEMENT

        // update expiration date TO BE IMPLEMENT

        while (true) {

            cout << "\n1. Update item name" << endl;
            cout << "2. Update item quantity" << endl;
            cout << "3. Update item category" << endl;
            cout << "4. Update item description" << endl;
            cout << "5. Done updating" << endl;
            cout << "Enter your choice: ";

            if (!(cin >> choice)) {
                cout << "\nInvalid input!" << endl;
                cout << "Please enter a number" << endl;
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                continue;
            }

            // find item id
            try {
                mysqlx::Session session("127.0.0.1", 33060, "root", "noelmehari1");
                mysqlx::Schema DB = session.getSchema("PantryPal");
                mysqlx::Table Item = DB.getTable("Item");

                ItemId = findItemId(Item, ItemName, UserId);
            }
            catch (const mysqlx::Error &err) {
                cerr << "\nConnection error: " << err.what() << endl;
            }

            if (choice == 1) { // update item name

                // get new item name
                while (true) {
                    cout << "Enter new item name (Maximum characters is 254): ";
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    getline(cin, newItemName);
                    if (!validateLength(newItemName)) {
                        cout << "\nItem name too long" << endl;
                    }
                    else if (newItemName == ItemName) {
                        break;
                    }
                    else if (itemExists(newItemName, UserId)) {
                        cout << "\nItem already exists" << endl;
                    }
                    else {
                        break;
                    }
                }

                // update item name
                try {
                    mysqlx::Session session("127.0.0.1", 33060, "root", "noelmehari1");
                    mysqlx::Schema DB = session.getSchema("PantryPal");
                    mysqlx::Table Item = DB.getTable("Item");

                    if (newItemName == ItemName) {
                        cout << "\nNo change was made" << endl;
                    }

                    else if (ItemId != -1) {
                        Item.update()
                            .set("ItemName", newItemName)
                            .where("ItemId = :item_id AND UserId = :user_id")
                            .bind("item_id", ItemId)
                            .bind ("user_id", UserId)
                            .execute();
                        cout << "\nItem name changed successfully!" << endl;
                        ItemName = newItemName;
                    }

                    else {
                        cout << "\nItem not found" << endl;
                    }
                }
                catch (const mysqlx::Error &err) {
                    cerr << "\nConnection error: " << err.what() << endl;
                }
            }

            else if (choice == 2) { // update item quantity

                // get new item quantity
                while (true) {
                    cout << "Enter new item quantity: ";
                    if (!(cin >> newItemQuant)) {
                        cout << "\nInvalid input!" << endl;
                        cout << "Please enter a number" << endl;
                        cin.clear();
                        cin.ignore(numeric_limits<streamsize>::max(), '\n');
                        continue;
                    }
                    break;
                }

                // update item quantity
                try {
                    mysqlx::Session session("127.0.0.1", 33060, "root", "noelmehari1");
                    mysqlx::Schema DB = session.getSchema("PantryPal");
                    mysqlx::Table Item = DB.getTable("Item");

                    if (newItemQuant == ItemQuant) {
                        cout << "\nNo change was made" << endl;
                    }

                    else if (ItemId != -1) {
                        Item.update()
                            .set("ItemQuant", newItemQuant)
                            .where("ItemId = :item_id and UserId = :user_id")
                            .bind("item_id", ItemId)
                            .bind("user_id", UserId)
                            .execute();
                        cout << "\nItem quantity changed successfully!" << endl;
                        ItemQuant = newItemQuant;
                    }

                    else {
                        cout << "\nItem not found" << endl;
                    }

                }
                catch (const mysqlx::Error &err) {
                    cerr << "\nConnection error: " << err.what() << endl;
                }
            }

            else if (choice == 3) {
                // update item category
                printCategories();
                while (true) {
                    cout << "Enter new category number: ";
                    if (!(cin >> CatId)) {
                        cin.clear();
                        cin.ignore(numeric_limits<streamsize>::max(), '\n');
                        cout << "Invalid input!" << endl;
                        cout << "Please enter a number" << endl;
                        continue;
                    }
                    if (!categoryExists(CatId)) {
                        cout << "Invalid choice" << endl;
                    }
                    break;
                }
            }

            else if (choice == 4) { // update item description

                // get item description
                // include case when new item desc same as before TO BE IMPLEMENT
                while (getline(cin, newItemDesc)) {
                    cout << "Enter new item description: ";
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    if (validateLength(ItemDesc)) {
                        break;
                    }
                    cout << "\nItem description too long" << endl;
                }

                //update item description
                try {
                    mysqlx::Session session("127.0.0.1", 33060, "root", "noelmehari1");
                    mysqlx::Schema DB = session.getSchema("PantryPal");
                    mysqlx::Table Item = DB.getTable("Item");

                    if (newItemDesc == ItemDesc) {
                        cout << "\nNo change was made" << endl;
                    }

                    else if (ItemId != -1) {
                        Item.update()
                            .set("ItemDesc", newItemDesc)
                            .where("ItemId = :item_id and UserId = :user_id")
                            .bind("item_id", ItemId)
                            .bind("user_id", UserId)
                            .execute();
                        cout << "\nItem description changed successfully!" << endl;
                        ItemDesc = newItemDesc;
                    }

                    else {
                        cout << "\nItem not found" << endl;
                    }

                }
                catch (const mysqlx::Error &err) {
                    cerr << "Connection error: " << err.what() << endl;
                }
            }

            else if (choice == 5) {
                break;
            }

            else {
                cout << "Invalid choice" << endl;
            }
        }

        while (true) {
            cout << "\n1. Update another item" << endl;
            cout << "2. Back to main menu" << endl;
            cout << "Enter your choice: ";

            if (!(cin >> option)) {
                cout << "\nInvalid input!" << endl;
                cout << "Please enter a number" << endl;
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                continue;
            }

            if (option != 1 && option != 2) {
                cout << "\nInvalid choice" << endl;
                continue;
            }

            break;
        }

        if (option == 1) {
            continue;
        }
        if (option == 2) {
            break;
        }
    }
}

void deleteItem(const int &UserId) {
    int option;
    char answer;
    string ItemName;

    while (true) {
        cout << "\n=== Delete Item ===\n";

        try {
            mysqlx::Session session("127.0.0.1", 33060, "root", "noelmehari1");
            mysqlx::Schema DB = session.getSchema("PantryPal");
            mysqlx::Table Item = DB.getTable("Item");

            auto result = Item.select("COUNT(*)")
                .where("UserID = :user_id")
                .bind("user_id", UserId)
                .execute();
            auto row = result.fetchOne();
            int count = row[0];

            if (count == 0) {
                cout << "\nYou have no items to delete" << endl;
                cout << "Heading back to main menu" << endl;
                break;
            }
        }
        catch (const mysqlx::Error &err) {
            cerr << "\nConnection error: " << err.what() << endl;
        }

        // get item name
        while (true) {
            cout << "Enter item name: ";
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            getline(cin, ItemName);
            if (itemExists(ItemName, UserId)) {
                break;
            }
            cout << "\nItem does not exist" << endl;
        }

        // confirm
        while (true) {
            cout << "\nDelete this item? (Y/N)" << endl;
            cin >> answer;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');

            if ((answer == 'Y') || (answer == 'y')) {
                // delete item
                try {
                    mysqlx::Session session("127.0.0.1", 33060, "root", "noelmehari1");
                    mysqlx::Schema DB = session.getSchema("PantryPal");
                    mysqlx::Table Item = DB.getTable("Item");

                    Item.remove()
                        .where("ItemName = :item_name AND UserID = :user_id")
                        .bind("item_name", ItemName)
                        .bind("user_id", UserId)
                        .execute();

                    cout << "\nItem deleted successfully!" << endl;
                }
                catch (const mysqlx::Error &err) {
                    cerr << "\nConnection error: " << err.what() << endl;
                }
                break;
            }

            if ((answer == 'N') || (answer == 'n')) {
                cout << "\nDeletion canceled" << endl;
                break;
            }

            cout << "\nInvalid answer" << endl;
        }

        while (true) {
            cout << "\n1. Delete another item" << endl;
            cout << "2. Back to main menu" << endl;
            cout << "Enter your choice: ";

            if (!(cin >> option)) {
                cout << "\nInvalid input!" << endl;
                cout << "Please enter a number" << endl;
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                continue;
            }

            if (option != 1 && option != 2) {
                cout << "\nInvalid choice" << endl;
                continue;
            }

            break;
        }

        if (option == 1) {
            continue;
        }

        if (option == 2) {
            break;
        }
    }
}

void deleteAllItems(const int &UserId) {
    char answer;
    string ItemName;

    while (true) {
        cout << "\n=== Delete All Items ===\n";

        try {
            mysqlx::Session session("127.0.0.1", 33060, "root", "noelmehari1");
            mysqlx::Schema DB = session.getSchema("PantryPal");
            mysqlx::Table Item = DB.getTable("Item");

            auto result = Item.select("COUNT(*)")
                .where("UserID = :user_id")
                .bind("user_id", UserId)
                .execute();
            auto row = result.fetchOne();
            int count = row[0];

            if (count == 0) {
                cout << "\nYou have no items to delete" << endl;
                cout << "Heading back to main menu" << endl;
                break;
            }
        }
        catch (const mysqlx::Error &err) {
            cerr << "\nConnection error: " << err.what() << endl;
        }

        // confirm
        while (true) {
            cout << "\nDelete all items? (Y/N)" << endl;
            cin >> answer;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');

            if ((answer == 'Y') || (answer == 'y')) {
                // delete all items
                try {
                    mysqlx::Session session("127.0.0.1", 33060, "root", "noelmehari1");
                    mysqlx::Schema DB = session.getSchema("PantryPal");
                    mysqlx::Table Item = DB.getTable("Item");

                    Item.remove()
                        .where("UserID = :user_id")
                        .bind("user_id", UserId)
                        .execute();

                    cout << "\nAll items deleted!" << endl;
                }
                catch (const mysqlx::Error &err) {
                    cerr << "\nConnection error: " << err.what() << endl;
                }
            }

            else if ((answer == 'N') || (answer == 'n')) {
                cout << "\nDeletion canceled" << endl;
            }

            else {
                cout << "\nInvalid answer" << endl;
            }
            break;
        }
        break;
    }
}

void viewCategories() {
    int option;

    try {
        mysqlx::Session session("127.0.0.1", 33060, "root", "noelmehari1");
        session.sql("USE PantryPal").execute();

        auto result = session.sql("SELECT CatId, CatName "
                                        "FROM Category "
                                        "ORDER BY CatId")
                                        .execute();

        cout << "\n=== Categories ===\n";

        auto row = result.fetchOne();

        if (!row) {
            cout << "\nNo categories available" << endl;
        }

        else {
            do {
                cout << row[0].get<int>() << "   " << row[1].get<string>() << endl;
            }
            while (row = result.fetchOne());
        }
    }
    catch (const mysqlx::Error &err) {
        cerr << "\nConnection error: " << err.what() << endl;
    }

    while (true) {
        cout << "\n1. Back to main menu" << endl;
        cout << "Enter your choice: ";

        if (!(cin >> option)) {
            cout << "Invalid input!" << endl;
            cout << "Please enter a number" << endl;
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }

        if (option == 1) {
            break;
        }

        cout << "Invalid choice" << endl;
    }
}

void mainMenu(const int &UserId) {

    int choice;

    while (true) {
        cout << "\n====== PantryPal ======\n";

        while (true) {
            cout << "\n1. View items" << endl;
            cout << "2. Add item" << endl;
            cout << "3. Update item" << endl;
            cout << "4. Delete items" << endl;
            cout << "5. Delete all items" << endl;
            cout << "6. View categories" << endl;
            cout << "7. Exit" << endl;
            cout << "Enter your choice: ";

            if (!(cin >> choice)) {
                cout << "Invalid input!" << endl;
                cout << "Please enter a number" << endl;
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                continue;
            }
            break;
        }

        if (choice == 1) {
            viewItems(UserId);
        }
        else if (choice == 2) {
            addItem(UserId);
        }
        else if (choice == 3) {
            updateItem(UserId);
        }
        else if (choice == 4) {
            deleteItem(UserId);
        }
        else if (choice == 5) {
            deleteAllItems(UserId);
        }
        else if (choice == 6) {
            viewCategories();
        }
        else if (choice == 7) {
            cout << "Goodbye!" << endl;
            break;
        }
        else {
            cout << "\nInvalid choice!" << endl;
        }
    }
}

void HomePage(const int &UserId, CurrentUser &Profile) {
    int userOption;

    while (true) {
        cout << "\n====== PantryPal ======\n";
        cout << "=======  Home Page  =======\n";

        cout << "\n1. View your Cart"
             << "\n2. View your profile"
             << "\n3. View Alerts"
             << "\n4. Sign Off"
             << "\n\nEnter your choice: ";

        if (!(cin >> userOption)) {
            cout << "\nInvalid input. Please enter a number.\n";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }

        if (userOption == 1) {
            mainMenu(UserId);
        }
        else if (userOption == 2) {
            int profileOption;

            while (true) {
                cout << "\n====== Profile ======\n";
                cout << "Username: " << Profile.getUsername() << endl;
                cout << "Email: " << Profile.getEmail() << endl;
                cout << "Created On " << Profile.getCreated_on() << endl;

                cout << "\n1. Back to Home Page" << endl;
                cout << "2. Edit Profile" << endl;
                cout << "Enter your choice: ";

                if (!(cin >> profileOption)) {
                    cout << "\nInvalid input. Please enter a number.\n";
                    cin.clear();
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    continue;
                }

                if (profileOption == 1) {
                    break;
                }
                else if (profileOption == 2) {
                    int editOption;

                    while (true) {
                        cout << "\n=== Editing Profile ===" << endl;
                        cout << "\n1. Edit Username"
                             << "\n2. Edit Email"
                             << "\n3. Edit Password"
                             << "\n4. Go back"
                             << "\nEnter your choice: ";

                        if (!(cin >> editOption)) {
                            cout << "\nInvalid input. Please enter a number.\n";
                            cin.clear();
                            cin.ignore(numeric_limits<streamsize>::max(), '\n');
                            continue;
                        }

                        if (editOption == 1) {
                            Profile.ChangeUsername();
                            cout << "\nUsername changed successfully!" << endl;
                        }
                        else if (editOption == 2) {
                            Profile.ChangeEmail();
                            cout << "\nEmail changed successfully!" << endl;
                        }
                        else if (editOption == 3) {
                            Profile.ChangePassword();
                            cout << "\nPassword changed successfully!" << endl;
                        }
                        else if (editOption == 4) {
                            break;
                        }
                        else {
                            cout << "\nInvalid choice. Try again.\n";
                        }
                    }
                }
                else {
                    cout << "\nInvalid choice. Try again.\n";
                }
            }
        }

        else if (userOption == 3) {
            cout << "\nNo alerts currently.\n";
            //Implement expiration alerts here
        }
        else if (userOption == 4) {
            cout << "See ya later " << Profile.getUsername() << endl;
            break;
        }
        else {
            cout << "\nInvalid choice. Try again.\n";
        }
    }
}




int main() {

    try {
        mysqlx::Session session("127.0.0.1", 33060, "root", "noelmehari1");  //Creating the user table locally

        session.sql("CREATE DATABASE IF NOT EXISTS PantryPal").execute();
        session.sql("USE PantryPal").execute();

        session.sql(
            "CREATE TABLE IF NOT EXISTS User ("
            "  Userid INT AUTO_INCREMENT PRIMARY KEY,"
            "  Username VARCHAR(255) NOT NULL,"
            "  created_on TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
            "  Email VARCHAR(255) UNIQUE NOT NULL,"
            "  Password VARCHAR(50) NOT NULL"
            ")"
        ).execute();

        session.sql(
            "CREATE TABLE IF NOT EXISTS Category ("
            "  CatId INT AUTO_INCREMENT PRIMARY KEY,"
            "  CatName VARCHAR(255) NOT NULL UNIQUE"
            ")"
        ).execute();

        session.sql(
            "INSERT IGNORE INTO Category (CatName) VALUES "
                "('Dairy'),"
                "('Fruits'),"
                "('Vegetables'),"
                "('Grains'),"
                "('Meat'),"
                "('Seafood'),"
                "('Beverages'),"
                "('Snacks'),"
                "('Frozen'),"
                "('Canned Goods')"
            ).execute();

        session.sql(
            "CREATE TABLE IF NOT EXISTS Item ("
            "  ItemId INT AUTO_INCREMENT PRIMARY KEY,"
            "  Userid int NOT NULL,"
            "  FOREIGN KEY (Userid) references User(Userid),"
            "  CatId int NOT NULL,"
            "  FOREIGN KEY (CatId) references Category(CatId),"
            "  ItemName VARCHAR(255) NOT NULL,"
            "  ItemQuant INT NOT NULL,"
            "  ItemDesc VARCHAR(255),"
            "  Added_on TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
            "  Exp_date TIMESTAMP NOT NULL"
            ")"
        ).execute();

        session.sql(
            "CREATE TABLE IF NOT EXISTS Expiration ("
            "  exp_alert INT PRIMARY KEY,"  // 0 or 1 (1:for true is expired and 0:for false is not yet expired)
            "  ItemId INT,"
            "  FOREIGN KEY (ItemId) references Item(ItemId),"
            "  Exp_date TIMESTAMP NOT NULL,"
            "  FOREIGN KEY (Exp_date) references Item(Exp_date),"
            "  is_dismissed INT," //if the item was deleted then this should equal 1, if not then it should equal 0
            "  Added_on TIMESTAMP NOT NULL,"
            "  FOREIGN KEY (Added_on) references Item(Added_on)"
            ")"
        ).execute();

        mysqlx::SqlResult result =
            session.sql("SELECT Userid, Username, created_on, Email, Password "
                        "FROM User "
                        "ORDER BY Userid DESC LIMIT 1")
                   .execute();


        // Quick check that SQL works:
        session.sql("SELECT 1").execute();

        std::cout << "Connected successfully!" << std::endl;

        session.close();
    }


    catch (const mysqlx::Error &err) {
        std::cerr << "Connection error: " << err.what() << std::endl;
    }


    int UserID;
    bool validpassword = false;
    bool validEmail = false;
    char UsersResponse;
    bool UsingApp = true;
    string Username, Email, Password, Created_on;


    while (UsingApp) {
        cout << "Welcome to PantryPal" << endl;
        cout << "Would you like to create an account? (Y/N)"  << endl;
        cin >> UsersResponse;
        if ((UsersResponse == 'Y') || (UsersResponse == 'y')) {
            cout << "Please create a username (Maximum characters is 254):" << endl;
            cin >> Username;

            while (Username.length() > 254) {
                cout << "Invalid username" << endl;
                cout << "Please create a username (Maximum characters is 254): ";
                cin >> Username;
            }

            PasswordValidation();
            getline(cin,Password);

            while (!validpassword) {
                bool hasDigit = false;
                bool hasUpper = false;
                bool hasLower = false;
                bool hasWhiteSpace = true;

                if (Password.length() < 10 || Password.length() > 50) {
                    validpassword = false;
                } else {

                    for (size_t i = 0; i < Password.size(); i++) {
                        if (isdigit(Password[i]))  hasDigit = true;
                        if (isupper(Password[i]))  hasUpper = true;
                        if (islower(Password[i]))  hasLower = true;
                        if (isspace(Password[i]))  hasWhiteSpace = false;
                    }

                    validpassword = hasDigit && hasUpper && hasLower && hasWhiteSpace;
                }

                if (!validpassword) {
                    cout << "Please enter a valid password: ";
                    getline(cin,Password);
                }
            }



            cout << "Please enter your email: " << endl;
            getline(cin,Email);

            while (!validEmail) {

                validEmail = true;


                if (Email.length() > 254) {
                    validEmail = false;}


                if (count(Email.begin(), Email.end(), '@') != 1) {
                    validEmail = false;}

                if (Email.find('.') == -1) {
                    validEmail = false;}


                size_t atPos = Email.find('@');
                if (atPos == 0 || atPos == -1) {
                    validEmail = false;}

                for (size_t i = 0; i < Email.size(); i++) {
                    if (isspace(Email[i])) {
                        validEmail = false;
                        break;}
                }

                if (validEmail) {
                    if (!(Email.ends_with("@gmail.com") ||
                          Email.ends_with("@yahoo.com") ||
                          Email.ends_with("@hotmail.com") ||
                          Email.ends_with("@outlook.com"))) {

                        validEmail = false;}
                }

                if (!validEmail) {
                    cout << "Invalid email\n";
                    cout << "Valid email example: johnsmith@outlook.com\n";
                    cout << "Please enter your email: ";
                    getline(cin,Email);}



                mysqlx::Session session("127.0.0.1", 33060, "root", "noelmehari1");

                mysqlx::Schema DB = session.getSchema("PantryPal");

                mysqlx::Table Users = DB.getTable("User");

                auto Read = Users
                .select("Email")
                .where("Email = :email" )
                .bind("email", Email)
                .execute();

                if (Read.count() > 0) {
                    cout << "Email already in use.\n";
                    validEmail = false;
                    cout << "Enter a different email: ";
                    cin >> Email;
                }

                else if (validEmail){
                    Users
                    .insert("Username", "Email", "Password")
                    .values(Username, Email, Password)
                    .execute();

                    UserID = findUserId(Users, Email);

                    auto result = Users.select("Email", "DATE_FORMAT (created_on, '%Y-%m-%d %H:%i:%s')")
                          .where("Email = :email")
                          .bind("email", Email)
                          .execute();

                    auto row = result.fetchOne(); //retrieving created on date
                    if (row) {
                        Created_on = row[1].get<string>();
                    }

                    CurrentUser Profile (Email, Password, Username, UserID, Created_on);

                    cout << "Account created successfully!" << endl;
                    cout << "Welcome to The Pantry Pal: " << Profile.getUsername() << endl;

                    HomePage(UserID, Profile);
                }

            }

            UsingApp = false;

        }



        else if (UsersResponse == 'N' || UsersResponse == 'n') {


            bool validlogin = false;
            string Username, Email, Password, Created_on;
            int UserId;

            cout << "Would you like to log into an existing account (Y/N)";
            cin >> UsersResponse;
            if (UsersResponse == 'Y' || UsersResponse == 'y') {

                mysqlx::Session session ("127.0.0.1", 33060, "root", "noelmehari1");

                mysqlx::Schema DB = session.getSchema("PantryPal");

                mysqlx::Table Users = DB.getTable("User");

                cout << "\nPlease enter your Email: ";
                cin >> Email;

                while (validlogin == false){

                    validlogin = true;

                    auto Read = Users
                    .select("Password")
                    .where("Email = :email")
                    .bind("email", Email)
                    .execute();

                    if (Read.count() == 0) {
                        cout << "Email not found. Please try again." << endl; //prompting for email login check
                        validlogin = false;
                        cout << "Enter your Email: ";
                        cin >> Email;
                    }


                    else {
                        cout << "Please enter your password: ";
                        cin >> Password;
                        mysqlx::Row row = Read.fetchOne();
                        string storedPassword = row[0].get<string>();

                        for (size_t i = 3; i > 0; i--) {
                            if (Password == storedPassword) {
                                cout << "Password is correct!" << endl;



                                auto result = Users.select("Username", "DATE_FORMAT (created_on, '%Y-%m-%d %H:%i:%s')")
                                .where("Email = :email")
                                .bind("email", Email)
                                .execute();

                                auto row = result.fetchOne();
                                if (row) {
                                    Username = row[0].get<string>();
                                    Created_on = row[1].get<string>();
                                }

                                UserId = findUserId(Users, Email);


                                CurrentUser Profile (Email, Password, Username, UserId, Created_on);



                                cout << "Account logged in successfully!" << endl;
                                cout << "Welcome back to The Pantry Pal: " << Profile.getUsername() << endl;

                                HomePage(UserId, Profile);

                                break;

                            }
                            else {
                                cout << "Password is incorrect!" << endl;
                                cout << "Please try again. (Attempts remaining: " << i - 1 << ")" << endl;
                                cout << "Please enter your password: ";
                                cin >> Password;
                                if (i == 1) {
                                    UsingApp = false;
                                    cerr << "Too many incorrect attempts. Please try again later." << endl;
                                    return 0;
                                }
                            }
                        }
                    }
                }

                UsingApp = false; //change later
            }
            else if (UsersResponse == 'N' || UsersResponse == 'n') {
                UsingApp = false;
            }
        }


        else {UsingApp = false;}
    }

    return 0;
}