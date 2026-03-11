//
// Created by Noel Mehari on 2/28/26.
//
//
// Created by Noel Mehari on 2/25/26.
//
#include <mysqlx/xdevapi.h>
#include <iostream>
#include <algorithm>
#include <string>
#include <string_view>
#include <cctype>
#include <limits>

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
        "Enter password here: ";
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

bool categoryExists(const string &CatName) {
    mysqlx::Session session("127.0.0.1", 33060, "root", "noelmehari1");
    mysqlx::Schema DB = session.getSchema("PantryPal");
    mysqlx::Table Category = DB.getTable("Category");

    auto Read = Category
    .select("CatName")
    .where("CatName = :cat_name")
    .bind("cat_name", CatName)
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

void viewCategories() {
    int option;

    try {
        mysqlx::Session session("127.0.0.1", 33060, "root", "noelmehari1");
        session.sql("USE PantryPal").execute();

        auto result = session.sql("SELECT CatName "
                                        "FROM Category ")
                                        .execute();

        cout << "\n=== Categories ===\n";

        auto row = result.fetchOne();

        if (!row) {
            cout << "\n No categories available" << endl;
        }

        else {
            do {
                cout << static_cast<int>(row[0]) << "   " << static_cast<string>(row[1]) << endl;
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

void viewItems(const int &UserId) {
    int option;

    try {
        mysqlx::Session session("127.0.0.1", 33060, "root", "noelmehari1");
        session.sql("USE PantryPal").execute();

        auto result = session.sql("SELECT ItemName, CatName, ItemQuant, ItemDesc, created_on, ExpDate,  "
                                        "FROM Item "
                                        "JOIN Category ON Item.CatId = Category.CatId "
                                        "JOIN Expiration ON Item.ItemId = Expiration.ItemId "
                                        "WHERE UserId = ?")
                                        .bind(UserId)
                                        .execute();

        cout << "\n=== Your Items ===\n";

        auto row = result.fetchOne();

        if (!row) {
            cout << "\n You have no items" << endl;
        }

        else {
            do {
                cout << "- Name: " << row[1].get<string>();
                cout << "  Category: " << row[2].get<string>() << endl;
                cout << "  Quantity:" << row[3].get<int>();
                if (!row[4].isNull()) {
                cout << "  Description:" << row[4].get<string>();
                }
                cout << "  Added on: " << row[5].get<string>();
                cout << "  Expiry date:" << row[6].get<string>();
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

void addItem(const int &UserId) {
    int ItemQuant, CatId, option;
    string ItemName, ItemDesc, ItemCategory, CatName;

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
        viewCategories();
        while (true) {
            cout << "Enter item category: ";
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            getline(cin, CatName);
            if (!itemExists(ItemName, UserId)) {
                cout << "Category does not exist" << endl;
            }
            else {
                CatId = findCatId(CatName);
                break;
            }
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

        // get expiration date TO BE IMPLEMENT

        // add item
        try {
            mysqlx::Session session("127.0.0.1", 33060, "root", "noelmehari1");
            mysqlx::Schema DB = session.getSchema("PantryPal");
            mysqlx::Table Item = DB.getTable("Item");

            Item.insert("UserId", "CatId", "ItemName", "ItemQuant", "ItemDesc")
                .values(UserId, CatId, ItemName, ItemQuant, ItemDesc)
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

    int choice, ItemId, ItemQuant, newItemQuant, option;
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
                /*cout << "Enter new item category << endl;
                 *while (getline(cin, ItemCat)
                 *if not exist
                 *1. enter another
                 *2. ?
                 *get CatID
                 *update
                 */
            }

            else if (choice == 4) { // update item description

                // get item description
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

int main() {
    try {
        mysqlx::Session session("127.0.0.1", 33060, "root", "noelmehari1");

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
            "  CatName VARCHAR(255) NOT NULL"
            ")"
        ).execute();

        session.sql(
            "INSERT INTO Category (CatName) VALUES "
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
            "  UserId INT,"
            "  FOREIGN KEY (Userid) references User(Userid),"
            "  CatId INT,"
            "  FOREIGN KEY (CatId) references Category(CatId),"
            "  ItemName VARCHAR(255) NOT NULL,"
            "  ItemQuant INT NOT NULL,"
            "  ItemDesc VARCHAR(255),"
            "  created_on TIMESTAMP DEFAULT CURRENT_TIMESTAMP"
            ")"
        ).execute();

        mysqlx::SqlResult result =
            session.sql("SELECT Userid, Username, created_on, Email, Password "
                        "FROM User "
                        "ORDER BY Userid DESC LIMIT 1")
                   .execute();


        auto row = result.fetchOne();
        if (row) {
            std::cout << "Wrote row: Userid=" << int(row[0])
                      << ", Username=" << std::string(row[1])
                      << ", created_on=" << std::string(row[2])
                      << ",Email=" <<std::string(row[3])
                      << ",Password=" << std::string(row[4])
                      << std::endl;
        }

        // Quick check that SQL works:
        session.sql("SELECT 1").execute();

        std::cout << "Connected successfully!" << std::endl;

        session.close();
    }


    catch (const mysqlx::Error &err) {
        std::cerr << "Connection error: " << err.what() << std::endl;
    }

    int UserId;
    bool validpassword = false;
    bool validEmail = false;
    char UsersResponse;
    bool UsingApp = true;
    string Username, Email, Password;


    while (UsingApp) {

        cout << "Welcome to PantryPal" << endl;
        cout << "Would you like to create an account? (Y/N)" << endl;
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
            cin >> Password;

            while (!validpassword) {
                bool hasDigit = false;
                bool hasUpper = false;
                bool hasLower = false;

                if (Password.length() < 10 || Password.length() > 50) {
                    validpassword = false;
                } else {

                    for (size_t i = 0; i < Password.size(); i++) {
                        if (isdigit(Password[i]))  hasDigit = true;
                        if (isupper(Password[i]))  hasUpper = true;
                        if (islower(Password[i]))  hasLower = true;}

                    validpassword = hasDigit && hasUpper && hasLower;
                }

                if (!validpassword) {
                    cout << "Please enter a valid password: ";
                    cin >> Password;}
            }



        cout << "Please enter your email: " << endl;
        cin >> Email;

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
                cin >> Email;}



            mysqlx::Session session("127.0.0.1", 33060, "root", "noelmehari1");

            mysqlx::Schema DB = session.getSchema("PantryPal");

            mysqlx::Table Users = DB.getTable("User");

            auto Read = Users
            .select("Email")
            .where("Email = :email")
            .bind("email", Email)
            .execute();

            if (Read.count() > 0) {
                cout << "Email already in use.\n";
                validEmail = false;
                cout << "Enter a different email: ";
                cin >> Email;
            }


            else {
                Users
                .insert("Username", "Email", "Password")
                .values(Username, Email, Password)
                .execute();

                UserId = findUserId(Users, Email);

            cout << "Account created successfully!" << endl;
            cout << "Welcome to The Pantry Pal: " << Username << endl;
            }

        }

       /*Item entity code goes here (
            Create
            Read
            Update
            Destroy
       */
            mainMenu(UserId);

      UsingApp = false;
    }


else if (UsersResponse == 'N' || UsersResponse == 'n') {
    while (UsingApp) {
        cout << "Would you like to log into an existing account (Y/N)";
        cin >> UsersResponse;
        if (UsersResponse == 'Y' || UsersResponse == 'y') {

            mysqlx::Session session ("127.0.0.1", 33060, "root", "noelmehari1");

            mysqlx::Schema DB = session.getSchema("PantryPal");

            mysqlx::Table Users = DB.getTable("User");

            cout << "\nPlease enter your Email: ";
            cin >> Email;

            auto Read = Users
            .select("Password")
            .where("Email = :email")
            .bind("email", Email)
            .execute();

            if (Read.count() == 0) {
                cout << "Email not found. Please try again." << endl;
            }
            else {
                cout << "Please enter your password: ";
                cin >> Password;
                mysqlx::Row row = Read.fetchOne();
                string storedPassword = row[0].get<string>();

                for (size_t i = 3; i > 0; i--) {
                    if (Password == storedPassword) {
                        cout << "Password is correct!" << endl;

                        UserId = findUserId(Users, Email);

                        cout << "Account logged in successfully!" << endl;
                        cout << "Welcome back to The Pantry Pal: " << Username << endl;
                        break;
                    }
                    else {
                        cout << "Password is incorrect!" << endl;
                        cout << "Please try again. (Attempts remaining: " << i << ")" << endl;
                        cout << "Please enter your password: ";
                        cin >> Password;
                        if (i == 1) {
                            UsingApp = false;
                        }
                    }
                }
            }

            mainMenu(UserId);

            UsingApp = false; //change later
        }
        else if (UsersResponse == 'N' || UsersResponse == 'n') {
            UsingApp = false;
        }
    }
}
    else {UsingApp = false;}
}

    return 0;
}