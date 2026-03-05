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

using std::cout;
using std::endl;
using std::string;
using std::cin;
using std::cerr;
using std::isspace;
using std::getline;

void PasswordValidation() {
    cout <<
        "Password must be between 10-50 characters\n"
        "Password must contain at least one number\n"
        "Password must contain at least one uppercase letter\n"
        "Password must contain at least one lowercase letter\n"
        "Enter password here: ";
}
void itemMenu() {
    cout << "1. Add items" << endl;
    cout << "2. Update items" << endl;
    cout << "3. Delete items" << endl;
    cout << "4. Retrieve items" << endl;
    cout << "Enter your choice: " << endl;
}

void updateItemMenu() {
    cout << "1. Update item name" << endl;
    cout << "2. Update item quantity" << endl;
    cout << "3. Update item category" << endl;
    cout << "4. Update item description" << endl;
    cout << "Enter your choice: " << endl;
}

void printOptions() {
    cout << "1. Do something else" << endl;
    cout << "2. Quit" << endl;
    cout << "Enter your option: " << endl;
}

bool validateLength(const string &name) {
    if (name.length() > 254) {
        return false;
    }
    return true;
}

bool itemNameExists(const string &ItemName, const int &UserId) {
    mysqlx::Session session("127.0.0.1", 33060, "root", "noelmehari1");
    mysqlx::Schema DB = session.getSchema("PantryPal");
    mysqlx::Table Item = DB.getTable("Item");

    auto Read = Item
    .select("ItemName")
    .where("ItemName = :item_name and UserId = :user_id") // where user id = user id
    .bind("item_name", ItemName)
    .bind("user_id", UserId)
    .execute();

    if (Read.count() > 0) {
        return true;
    }

    return false;
}

bool validateQuantity(const int &ItemQuant) {
    if (ItemQuant >=0) {
        return true;
    }
    return false;
}

int findItemId(mysqlx::Table &Item, const string &ItemName) {
    auto result = Item.select("ItemID")
    .where("ItemName = :item_name") // and user id = user id
    .bind("item_name", ItemName)
    .execute();

    auto row = result.fetchOne();

    if (row) {
        return row[0];
    }
    return -1;
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


    bool validpassword = false;
    bool validEmail = false;
    char UsersResponse;
    bool UsingApp = true;
    string Username, Email, Password, ItemName, ItemDesc, newItemName;
    int choice, ItemQuant, option, UserId;
    int ItemId = -1;
    bool validChoice = false;

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
                }

                else {

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
                    cin >> Email;
                }



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
            itemMenu();
            cin >> choice;

            while (!validChoice) {

                if (choice == 1) { // add item

                    // get item name
                    cout << "Enter item name (Maximum characters is 254): " << endl;
                    cin.ignore();
                    while (getline(cin, ItemName)) {
                        if (!validateLength(ItemName)) {
                            cout << "Item name too long" << endl;
                        }
                        else if (itemNameExists(ItemName, UserId)) {
                            cout << "Item already exists" << endl;
                        }
                        else {
                            break;
                        }
                        cout << "Enter item name (Maximum characters is 254): ";
                    }

                    // get item quantity
                    cout << "Enter item quantity: " << endl;
                    while (cin >> ItemQuant) {
                        if (validateQuantity(ItemQuant)) {
                            break;
                        }
                        cout << "Invalid number" << endl;
                        cout << "Enter item quantity: " << endl;
                    }

                    // get item category
                    /*cout << "Enter item category: " << endl;
                     *if not exist
                     *1. create new
                     *2. enter other category
                     *find cat id
                     *CatId = findCatId(Category, ItemCat)
                     */

                    // get item description
                    cout << "Enter item description (Press Enter to skip): " << endl;
                    cin.ignore();
                    while (getline(cin, ItemDesc)) {
                        if (ItemDesc == "") {
                            cout << "Description skipped" << endl;
                            break;
                        }
                        if (validateLength(ItemDesc)) {
                            break;
                        }
                        cout << "Item description too long" << endl;
                        cout << "Enter item description (Press Enter to skip): " << endl;
                    }

                    // add item
                    try {
                        mysqlx::Session session("127.0.0.1", 33060, "root", "noelmehari1");
                        mysqlx::Schema DB = session.getSchema("PantryPal");
                        mysqlx::Table Item = DB.getTable("Item");

                        Item.insert("UserId", "ItemName", "ItemQuant", "ItemDesc")
                            .values(UserId, ItemName, ItemQuant, ItemDesc)
                            .execute(); // missing catid

                        cout << "Item added successfully!" << endl;
                    }
                    catch (const mysqlx::Error &err) {
                        cerr << "Connection error: " << err.what() << endl;
                    }
                }

                else if (choice == 2) { // update item

                    // get item name
                    cout << "Enter item name: " << endl;
                    cin.ignore();
                    while (getline(cin, ItemName)) {
                        if (itemNameExists(ItemName, UserId)) {
                            break;
                        }
                        cout << "Item does not exist" << endl;
                        cout << "Enter a different item name (Maximum characters is 254): ";
                    }

                    updateItemMenu();

                    while (cin >> choice) {
                        if (choice == 1) { // update item name

                            // get new item name
                            cout << "Enter new item name (Maximum characters is 254): ";
                            cin.ignore();
                            while (getline(cin, newItemName)) {
                                if (!validateLength(newItemName)) {
                                    cout << "Item name too long" << endl;
                                }
                                else if (itemNameExists(newItemName, UserId)) {
                                    cout << "Item already exists" << endl;
                                }
                                else {
                                    break;
                                }
                                cout << "Enter new item name (Maximum characters is 254): ";
                            }

                            // find item id and update item name
                            try {
                                mysqlx::Session session("127.0.0.1", 33060, "root", "noelmehari1");
                                mysqlx::Schema DB = session.getSchema("PantryPal");
                                mysqlx::Table Item = DB.getTable("Item");

                                ItemId = findItemId(Item, ItemName);

                                if (ItemId != -1) {
                                    Item.update()
                                        .set("ItemName", newItemName)
                                        .where("ItemId = :item_id AND UserId = :user_id")
                                        .bind("item_id", ItemId)
                                        .bind ("user_id", UserId)
                                        .execute();
                                    cout << "Item name changed successfully!" << endl;
                                }

                                else {
                                    cout << "Item not found" << endl;
                                }
                            }
                            catch (const mysqlx::Error &err) {
                                cerr << "Connection error: " << err.what() << endl;
                            }

                            break;
                        }

                        if (choice == 2) { // update item quantity

                            // get new item quantity
                            cout << "Enter new item quantity: ";
                            while (cin >> ItemQuant) {
                                if (validateQuantity(ItemQuant) == true) {
                                    break;
                                }
                                cout << "Invalid number" << endl;
                                cout << "Enter new item name quantity: ";
                            }

                            // find item id and update item quantity
                            try {
                                mysqlx::Session session("127.0.0.1", 33060, "root", "noelmehari1");
                                mysqlx::Schema DB = session.getSchema("PantryPal");
                                mysqlx::Table Item = DB.getTable("Item");

                                ItemId = findItemId(Item, ItemName);

                                if (ItemId != -1) {
                                    Item.update()
                                        .set("ItemQuant", ItemQuant)
                                        .where("ItemId = :item_id and UserId = :user_id")
                                        .bind("item_id", ItemId)
                                        .bind("user_id", UserId)
                                        .execute();
                                    cout << "Item quantity changed successfully!" << endl;
                                }

                                else {
                                    cout << "Item not found" << endl;
                                }
                            }
                            catch (const mysqlx::Error &err) {
                                cerr << "Connection error: " << err.what() << endl;
                            }

                            break;
                        }

                        if (choice == 3) { // update item category
                            /*find itemid
                             *ItemId = findItemId(Item, ItemName)
                             *get catid
                             *cout << "Enter new item category << endl;
                             *while (getline(cin, ItemCat)
                             *check if length is valid;
                             */
                        }

                        if (choice == 4) { // update item description

                            // get item description
                            cout << "Enter new item description: ";
                            cin.ignore();
                            while (getline(cin, ItemDesc)) {
                                if (validateLength(ItemDesc)) {
                                    break;
                                }
                                cout << "Item description too long" << endl;
                                cout << "Enter new item description: " << endl;
                            }

                            // find item description and update item description
                            try {
                                mysqlx::Session session("127.0.0.1", 33060, "root", "noelmehari1");
                                mysqlx::Schema DB = session.getSchema("PantryPal");
                                mysqlx::Table Item = DB.getTable("Item");

                                if (ItemId != -1) {
                                    Item.update()
                                        .set("ItemDesc", ItemDesc)
                                        .where("ItemId = :item_id and UserId = :user_id")
                                        .bind("item_id", ItemId)
                                        .bind("user_id", UserId)
                                        .execute();
                                    cout << "Item description changed successfully!" << endl;
                                }

                                else {
                                    cout << "Item not found" << endl;
                                }
                            }
                            catch (const mysqlx::Error &err) {
                                cerr << "Connection error: " << err.what() << endl;
                            }

                            break;
                        }

                        cout << "Invalid choice" << endl;
                        updateItemMenu();
                    }
                }

                /*choice 3: delete an item, delete all items
                 *choice 4: retrieve an item, retrieve all items
                 */

                else {
                    cout << "Invalid choice" << endl;
                    itemMenu();
                    cin >> choice;
                    continue;
                }

                printOptions();
                while (cin >> option) {
                    if (option == 1) {
                        itemMenu();
                        cin >> choice;
                        break;
                    }
                    if (option == 2) {
                        validChoice = true;
                        break;
                    }
                    cout << "Invalid option" << endl;
                    printOptions();
                }
            }
            UsingApp = false;
        }

        else if ((UsersResponse == 'N') || (UsersResponse == 'n')) {
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