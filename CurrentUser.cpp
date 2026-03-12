//
// Created by Noel Mehari on 2/25/26.
//
#include "CurrentUser.h"
#include <ctime>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <mysqlx/xdevapi.h>
#include <limits>

using namespace::std;



CurrentUser::CurrentUser(string email, string password, string username, int user_id, string created_on) {
Email = email;
Password = password;
 Username = username;
 UserId = user_id;
 this->created_on = created_on;

}

void CurrentUser::ChangeEmail() {
 string NewEmail;
 bool validEmail = false;


 cout << "Please enter your New Email: " << endl;
 cin.ignore(numeric_limits<streamsize>::max(), '\n');
 getline(cin,NewEmail);

 while (!validEmail) {
  validEmail = true;


  if (NewEmail.length() > 254) {
   validEmail = false;}


  if (count(NewEmail.begin(), NewEmail.end(), '@') != 1) {
   validEmail = false;}

  if (NewEmail.find('.') == -1) {
   validEmail = false;}


  size_t atPos = NewEmail.find('@');
  if (atPos == 0 || atPos == -1) {
   validEmail = false;}

  for (size_t i = 0; i < NewEmail.size(); i++) {
   if (isspace(NewEmail[i])) {
    validEmail = false;
    break;}
  }

  if (validEmail) {
   if (!(NewEmail.ends_with("@gmail.com") ||
         NewEmail.ends_with("@yahoo.com") ||
         NewEmail.ends_with("@hotmail.com") ||
         NewEmail.ends_with("@outlook.com"))) {

    validEmail = false;}
  }

  if (!validEmail) {
   cout << "Invalid email\n";
   cout << "Valid email example: johnsmith@outlook.com\n";
   cout << "Please enter your new email: ";
   getline(cin,NewEmail);}



  mysqlx::Session session("127.0.0.1", 33060, "root", "noelmehari1");

  mysqlx::Schema DB = session.getSchema("PantryPal");

  mysqlx::Table Users = DB.getTable("User");

  auto Read = Users
  .select("Email")
  .where("Email = :email" )
  .bind("email", NewEmail)
  .execute();

  if (Read.count() > 0) {
   cout << "Email already in use.\n";
   validEmail = false;
   cout << "Enter a different email: ";
   cin >> NewEmail;
  }
 }

  try {
   mysqlx::Session session("127.0.0.1", 33060, "root", "noelmehari1");  //Creating the user table locally

   session.sql("CREATE DATABASE IF NOT EXISTS PantryPal").execute();
   session.sql("USE PantryPal").execute();

   session.sql("UPDATE User "
               "SET Email = ? "
               "WHERE Userid = ?")

   .bind(NewEmail)
   .bind(UserId)
   .execute();

   Email = NewEmail;
  }

  catch (const mysqlx::Error &err) {
   std::cerr << "Connection error: " << err.what() << std::endl;
  }

}


void CurrentUser::ChangePassword() {
 string NewPassword, OldPassword;
 bool validpassword = false;

 cout << "Enter current Password here: ";
 cin.ignore(numeric_limits<streamsize>::max(), '\n');
 getline(cin,OldPassword);


 for (size_t i = 3; i > 0; i--) {
  if (Password == OldPassword) {
   cout << "Password is correct!\n\n" << endl;
   break;
  }

  else {
   cout << "Password is incorrect!" << endl;
   cout << "Please try again. (Attempts remaining: " << i - 1 << ")" << endl;
   cout << "Please enter your password: ";
   getline(cin,OldPassword);
   if (i == 1) {
    cerr << "Too many incorrect attempts. Please try again later." << endl;
    break;
   }
  }
 }

   PasswordValidation();
   getline(cin,NewPassword);

   while (!validpassword) {
    bool hasDigit = false;
    bool hasUpper = false;
    bool hasLower = false;
    bool hasWhiteSpace = true;

    if (NewPassword.length() < 10 || NewPassword.length() > 50) {
     validpassword = false;
    } else {

     for (size_t i = 0; i < NewPassword.size(); i++) {
      if (isdigit(NewPassword[i]))  hasDigit = true;
      if (isupper(NewPassword[i]))  hasUpper = true;
      if (islower(NewPassword[i]))  hasLower = true;
      if (isspace(NewPassword[i]))  hasWhiteSpace = false;
     }

     validpassword = hasDigit && hasUpper && hasLower && hasWhiteSpace;
    }

    if (!validpassword) {
     cout << "Please enter a valid password: ";
     getline(cin,NewPassword);
    }
   }


   try {
    mysqlx::Session session("127.0.0.1", 33060, "root", "noelmehari1");

    session.sql("CREATE DATABASE IF NOT EXISTS PantryPal").execute();
    session.sql("USE PantryPal").execute();

    session.sql("UPDATE User "
                "SET Password = ? "
                "WHERE Userid = ?")

    .bind(NewPassword)
    .bind(UserId)
    .execute();

    Password = NewPassword;
   }

   catch (const mysqlx::Error &err) {
    std::cerr << "Connection error: " << err.what() << std::endl;
   }
  }


  void CurrentUser::ChangeUsername() {
   string NewUsername;

   cout << "Enter new Username here: ";
   cin.ignore(numeric_limits<streamsize>::max(), '\n');
   getline(cin,NewUsername);

   try {
    mysqlx::Session session("127.0.0.1", 33060, "root", "noelmehari1");  //Creating the user table locally

    session.sql("CREATE DATABASE IF NOT EXISTS PantryPal").execute();
    session.sql("USE PantryPal").execute();

    session.sql("UPDATE User "
                "SET Username = ? "
                "WHERE Userid = ?")

    .bind(NewUsername)
    .bind(UserId)
    .execute();

    Username = NewUsername;
   }



   catch (const mysqlx::Error &err) {
    std::cerr << "Connection error: " << err.what() << std::endl;
   }
  }


  int CurrentUser::getUserId() {
   return UserId;
  }

  string CurrentUser::getCreated_on() {

   return created_on;
  }

  string CurrentUser::getPassword() {
   return Password;
  }

  string CurrentUser::getUsername() {
   return Username;
  }

  string CurrentUser::getEmail() {
   return Email;
  }

  void CurrentUser::PasswordValidation() {

   cout <<
       "Password must be between 10-50 characters\n"
       "Password must contain at least one number\n"
       "Password must contain at least one uppercase letter\n"
       "Password must contain at least one lowercase letter\n"
       "Password may not contain any Special characters\n"
       "Enter new password here: ";
  }
