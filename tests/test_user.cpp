#include <gtest/gtest.h>
#include "user.hpp"
#include "user_manager.hpp"

using namespace digipets;

// ---- User tests -----------------------------------------------------------

TEST(UserTest, PasswordHashNotPlaintext) {
    User u("alice", "secret");
    // The JSON representation must not contain the raw password
    auto j = u.to_json();
    EXPECT_NE(j.at("password_hash").get<std::string>(), "secret");
}

TEST(UserTest, PasswordVerifyCorrect) {
    User u("bob", "correcthorse");
    EXPECT_TRUE(u.verify_password("correcthorse"));
}

TEST(UserTest, PasswordVerifyWrong) {
    User u("bob", "correcthorse");
    EXPECT_FALSE(u.verify_password("wrong"));
}

TEST(UserTest, SaltIsUnique) {
    User u1("alice", "pass");
    User u2("alice", "pass");
    // Same username+password should produce different hashes due to unique salts
    auto j1 = u1.to_json();
    auto j2 = u2.to_json();
    EXPECT_NE(j1.at("salt").get<std::string>(),
              j2.at("salt").get<std::string>());
    EXPECT_NE(j1.at("password_hash").get<std::string>(),
              j2.at("password_hash").get<std::string>());
}

TEST(UserTest, RoundTripSerialization) {
    User original("charlie", "mypassword");
    auto j        = original.to_json();
    User restored = User::from_json(j);

    EXPECT_EQ(original.get_id(),       restored.get_id());
    EXPECT_EQ(original.get_username(), restored.get_username());
    // Password verification must still work after deserialization
    EXPECT_TRUE(restored.verify_password("mypassword"));
    EXPECT_FALSE(restored.verify_password("wrong"));
}

// ---- InputValidation tests ------------------------------------------------

TEST(InputValidation, ValidUsername) {
    EXPECT_TRUE(is_valid_username("alice"));
    EXPECT_TRUE(is_valid_username("Alice_1"));
    EXPECT_TRUE(is_valid_username("abc"));                   // min length 3
    EXPECT_TRUE(is_valid_username(std::string(32, 'a')));    // max length 32
}

TEST(InputValidation, InvalidUsername) {
    EXPECT_FALSE(is_valid_username("ab"));                   // too short
    EXPECT_FALSE(is_valid_username(std::string(33, 'a')));   // too long
    EXPECT_FALSE(is_valid_username("alice bob"));            // space
    EXPECT_FALSE(is_valid_username("alice!"));               // special char
    EXPECT_FALSE(is_valid_username(""));
}

TEST(InputValidation, ValidPassword) {
    EXPECT_TRUE(is_valid_password("12345678"));              // min 8 chars
    EXPECT_TRUE(is_valid_password(std::string(128, 'x')));  // max 128 chars
}

TEST(InputValidation, InvalidPassword) {
    EXPECT_FALSE(is_valid_password("short"));               // < 8 chars
    EXPECT_FALSE(is_valid_password(std::string(129, 'x'))); // > 128 chars
    EXPECT_FALSE(is_valid_password(""));
}

// ---- UserManager + session tests -----------------------------------------

TEST(UserManagerTest, RegisterAndAuthenticate) {
    UserManager mgr;
    auto uid = mgr.register_user("dave", "password1");
    ASSERT_TRUE(uid.has_value());

    auto auth_uid = mgr.authenticate("dave", "password1");
    ASSERT_TRUE(auth_uid.has_value());
    EXPECT_EQ(*uid, *auth_uid);
}

TEST(UserManagerTest, DuplicateRegistrationFails) {
    UserManager mgr;
    EXPECT_TRUE(mgr.register_user("eve", "password1").has_value());
    EXPECT_FALSE(mgr.register_user("eve", "password2").has_value());
}

TEST(UserManagerTest, WrongPasswordFails) {
    UserManager mgr;
    mgr.register_user("frank", "goodpass");
    EXPECT_FALSE(mgr.authenticate("frank", "badpass").has_value());
}

TEST(UserManagerTest, SessionCreateAndValidate) {
    UserManager mgr;
    auto uid = mgr.register_user("grace", "pa$$word");
    ASSERT_TRUE(uid.has_value());

    std::string token = mgr.create_session(*uid);
    EXPECT_FALSE(token.empty());
    EXPECT_EQ(token.size(), 64u);   // 32 bytes → 64 hex chars

    auto validated = mgr.validate_session(token);
    ASSERT_TRUE(validated.has_value());
    EXPECT_EQ(*validated, *uid);
}

TEST(UserManagerTest, RevokedSessionIsInvalid) {
    UserManager mgr;
    auto uid  = mgr.register_user("heidi", "pa$$word!");
    auto tok  = mgr.create_session(*uid);

    mgr.revoke_session(tok);
    EXPECT_FALSE(mgr.validate_session(tok).has_value());
}

TEST(UserManagerTest, UnknownTokenIsInvalid) {
    UserManager mgr;
    EXPECT_FALSE(mgr.validate_session("deadbeef").has_value());
}

TEST(UserManagerTest, Persistence) {
    const std::string file = "test_users.json";
    {
        UserManager mgr;
        mgr.register_user("ivan", "ivanpass1");
        mgr.register_user("judy", "judypass2");
        mgr.save_to_file(file);
    }
    {
        UserManager mgr;
        ASSERT_TRUE(mgr.load_from_file(file));
        EXPECT_TRUE(mgr.authenticate("ivan", "ivanpass1").has_value());
        EXPECT_TRUE(mgr.authenticate("judy", "judypass2").has_value());
        EXPECT_FALSE(mgr.authenticate("ivan", "wrongpass").has_value());
    }
    std::remove(file.c_str());
}
