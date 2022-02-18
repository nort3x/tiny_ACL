#include <stdio.h>
#include <string.h>
#include "constants.h"
#include "seridog.h"
/*
 * architecture of this unit is a nested method calls which all return bool (trampoline)
 */

// used as splitter to divide input to related code paths
inline bool analyze_input(int, char **);

int main(int argc, char **args) {

//     just controlling IO
    bool op_res = analyze_input(argc, args);
    printf(op_res ? "Success\n" : "Error: %s\n", ERROR_REASON);
    return op_res ? 0 : -1;

}


#define compare_and_invoke(str1, str2, op) if(strcmp(str1,str2) == 0) op

inline bool add_user(int argc, char **argv);

inline bool set_domain(int argc, char **argv);

inline bool authenticate(int argc, char **argv);

inline bool domain_info(int argc, char **argv);

inline bool set_type(int argc, char **argv);

inline bool type_info(int argc, char **argv);

inline bool add_access(int argc, char **argv);

inline bool can_access(int argc, char **argv);

bool analyze_input(int argc, char *argv[]) {

    // ignore the first arg and expect more
    if (argc <= 1) return_false_with(ILLEGAL_COUNT);

    char *indicator_arg = argv[1];

    compare_and_invoke(indicator_arg, "AddUser", return add_user(argc, argv));
    compare_and_invoke(indicator_arg, "Authenticate", return authenticate(argc, argv));
    compare_and_invoke(indicator_arg, "SetDomain", return set_domain(argc, argv));
    compare_and_invoke(indicator_arg, "DomainInfo", return domain_info(argc, argv));
    compare_and_invoke(indicator_arg, "SetType", return set_type(argc, argv));
    compare_and_invoke(indicator_arg, "TypeInfo", return type_info(argc, argv));
    compare_and_invoke(indicator_arg, "AddAccess", return add_access(argc, argv));
    compare_and_invoke(indicator_arg, "CanAccess", return can_access(argc, argv));


    return_false_with(UNKNOWN_INPUT)
}

bool add_user(int argc, char **argv) {
    seridog_load_user_context();
    if (argc < 3) return_false_with(ILLEGAL_COUNT);
    User to_add_user = {argv[2], argv[3]};
    if (to_add_user.username[0] == '\0') return_false_with("username missing")
    if(argc == 3)
        to_add_user.password="";
    bool ans = seridog_add_user(to_add_user);
    seridog_close_user_context();
    return ans;
}

bool authenticate(int argc, char **argv) {
    seridog_load_user_context();
    if (argc < 3) return_false_with(ILLEGAL_COUNT);
    User to_check_user = {argv[2], argv[3]};
    if (to_check_user.username[0] == '\0') return_false_with("username is mandatory")
    if(argc == 3) to_check_user.password = "";
    bool ans = seridog_check_authentication(to_check_user);
    seridog_close_user_context();
    return ans;
}

bool set_domain(int argc, char **argv) {
    seridog_load_user_context();
    seridog_load_domain_context();
    if (argc < 3) return_false_with(ILLEGAL_COUNT);
    char *user = argv[2];
    char *domain = argv[3];
    if(argc == 3 || domain[0] == '\0'){
        seridog_close_domain_context();
        seridog_close_user_context();
        return_false_with("missing domain");
    }
    if (user[0] == '\0'){
        seridog_close_domain_context();
        seridog_close_user_context();
        return_false_with("no such user")
    }
    bool ans = seridog_add_user_to_domain(user, domain);
    seridog_close_domain_context();
    seridog_close_user_context();
    return ans;
}

bool domain_info(int argc, char **argv) {
    seridog_load_domain_context();
    if (argc < 3) return_false_with(ILLEGAL_COUNT);
    char *domain = argv[2];
    if (domain[0] == '\0') return_false_with("missing domain")


    int count;
    char **userNames;
    seridog_get_domain_info(&userNames, &count, domain);

    if (count == -1) goto clean_return;

    for (int i = 0; i < count - 1; ++i) {
        printf("%s\n", userNames[i]);
    }
    free(userNames[0]); // because it is the actual beginning of the buffer others are just tokens!
    free(userNames); // yeah, this is array of pointers which is good to be freed aswell

    clean_return:
    seridog_close_domain_context();
    exit(0);
    return true;
}

bool set_type(int argc, char **argv) {
    seridog_load_type_context();
    if (argc < 3) return_false_with(ILLEGAL_COUNT);
    char *type = argv[2];
    char *object = argv[3];
    if(argc == 3 || type[0] == '\0'){
        seridog_close_type_context();
        return_false_with("missing type");
    }
    bool ans = seridog_add_object_to_type(object, type);
    seridog_close_type_context();
    return ans;


}

bool type_info(int argc, char **argv) {
    seridog_load_type_context();
    char *type = argv[2];
    if (type == NULL || type[0] == '\0') return_false_with("missing type")


    int count;
    char **objects;
    seridog_get_type_info(&objects, &count, type);

    if (count == -1) goto clean_return;

    for (int i = 0; i < count - 1; ++i) {
        printf("%s\n", objects[i]);
    }
    free(objects[0]); // because it is the actual beginning of the buffer others are just tokens!
    free(objects); // yeah, this is array of pointers which is good to be freed aswell

    clean_return:
    seridog_close_type_context();
    exit(0);
    return true;
}

bool add_access(int argc, char **argv) {
    seridog_load_access_context();
//    if (argc < 5) return_false_with(ILLEGAL_COUNT);
    char *op = argv[2];
    char *domain = argv[3];
    char *type = argv[4];
    if (type == NULL || type[0] == '\0') return_false_with("missing type")
    if (domain == NULL || domain[0] == '\0') return_false_with("missing domain")
    if (op == NULL || op[0] == '\0') return_false_with("missing operation")

    bool ans = seridog_add_access(op, (DomainTypeTuple) {type, domain});
    seridog_close_access_context();
    return ans;
}

bool can_access(int argc, char **argv) {
    seridog_load_access_context();
    seridog_load_domain_context();
    seridog_load_user_context();
    seridog_load_type_context();
    if (argc < 5) return_false_with(ILLEGAL_COUNT);
    char *op = argv[2];
    char *user = argv[3];
    char *object = argv[4];
    if (op[0] == '\0') return_false_with("missing operation")
    if (user[0] == '\0') return_false_with("missing user")
    if (object[0] == '\0') return_false_with("missing object")

    bool  ans = canAccess(user,object,op);


    seridog_close_access_context();
    seridog_close_domain_context();
    seridog_close_user_context();
    seridog_close_type_context();
    if(!ans) return_false_with("access denied")
    return ans;
}
