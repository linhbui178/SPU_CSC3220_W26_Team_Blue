//
// Created by Noel Mehari on 2/25/26.
//

#ifndef SPU_CSC3220_W26_TEAM_BLUE_TESTING_H
#define SPU_CSC3220_W26_TEAM_BLUE_TESTING_H
#endif //SPU_CSC3220_W26_TEAM_BLUE_TESTING_H

#include <string>
#include <ctime>
using std::string;


class CurrentUser {

private:
    int UserId;
    string created_on;
    string Password;

public:
    CurrentUser(string email, string password, string username, int user_id, string created_on);
    string Username;
    string Email;
    void ChangeEmail();
    void ChangePassword();
    void ChangeUsername();
    int getUserId();
    string getCreated_on();
    string getPassword();
    string getUsername();
    string getEmail();
    void PasswordValidation();

};

