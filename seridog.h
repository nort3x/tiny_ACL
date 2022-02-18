//
// Created by human on 2/16/22.
//

#ifndef SECURITY_SERIDOG_H
#define SECURITY_SERIDOG_H

#include "constants.h"
#include "stdlib.h"
#include "string_support.h"


#define user_context_loc "user_context.cv"
FILE *user_context;

#define BUFF_SIZE (1024*1024)


inline bool is_same_user_password(char *buff, User u);

inline bool is_same_user(char *buff, char *username);

inline void serialize_user(User u, char *ptr);


/******* user context *******/
void seridog_close_user_context() {
    if (user_context != NULL) {
        fflush(user_context);
        fclose(user_context);
    }
}

bool seridog_load_user_context() {
    bool first_try = true;
    LOAD:
    user_context = fopen(user_context_loc, "r+");
    // if we could not open it , it's probably because it doesn't exist so we create it
    if (user_context == NULL && first_try) {
        first_try = false; // this flag is used to make sure we don't make a loop and fail fast
        user_context = fopen(user_context_loc, "ab+");
        fclose(user_context);
        goto LOAD;
    }
    if (user_context == NULL) return_false_with("could not load user context")
    return true;
}

bool seridog_add_user(User u) {
    fseek(user_context, 0, SEEK_SET); // go to beginning of the file
    char buff[BUFF_SIZE];
    while (!feof(user_context)) {
        memset(buff, 0, BUFF_SIZE);
        char *read_buff = fgets(buff, BUFF_SIZE, user_context);
        if (read_buff == NULL) break; // because there is nothing to read else
        if (is_same_user(read_buff, u.username)) return_false_with("user exist");
    }
    memset(buff, 0, BUFF_SIZE);
    serialize_user(u, buff);
    if (fprintf(user_context, "%s\n", buff) != strlen(buff) + 1) return_false_with("could not persist user")
    return true;
}

bool seridog_check_authentication(User u) {
    fseek(user_context, 0, SEEK_SET); // go to beginning of the file
    char buff[BUFF_SIZE];
    bool found_user = false;
    while (!feof(user_context)) {
        memset(buff, 0, BUFF_SIZE);
        char *read_buff = fgets(buff, BUFF_SIZE, user_context);
        if (read_buff == NULL) break; // because there is nothing to read else
        if (is_same_user(read_buff, u.username)) {
            found_user = true;
            break;
        };
    }
    if (!found_user) return_false_with("no such user")

    if (!is_same_user_password(buff, u)) return_false_with("bad password");

    return true;
}

bool is_user_exist(char *username) {
    fseek(user_context, 0, SEEK_SET);
    char *buff;
    while ((buff = read_next_line_in_heap(user_context)) != NULL) {
        if (is_same_user(buff, username)) {
            free(buff);
            return true;
        }
        free(buff);
    }
    return false;
}

// hail to CV format: {"username","password"} --> username,password
void serialize_user(User u, char *ptr) {
    int len_username = strlen(u.username);
    int len_password = strlen(u.password);

    memcpy(ptr, u.username, len_username);
    ptr[len_username] = ',';
    memcpy(ptr + len_username + 1, u.password, len_password);
    ptr[len_username + len_password + 1] = '\0';
}

bool is_same_user_password(char *buff, User u) {
    char test_buff[BUFF_SIZE];
    int buff_str_len = strlen(buff);
    if (buff[buff_str_len - 1] == '\n') buff[buff_str_len - 1] = '\0';
    serialize_user(u, test_buff);
    bool is_equal = strcmp(buff, test_buff) == 0;
    if (!is_equal) return_false_with("user password doesn't match")
    return is_equal;
}

bool is_same_user(char *buff, char *username) {
    char *_index = index(buff, ',');
    if (_index == NULL) return_false_with("malformed buffer, in user check")

    int buffer_username_len = _index - buff; // old trick
    int username_len = strlen(username);

    if (username_len != buffer_username_len || strncmp(buff, username, username_len) != 0) return_false_with(
            "user doesn't match");

    return true;
}

/******* domain context *******/
#define domain_context_loc "domain_context.cv"
FILE *domain_context;


void seridog_close_domain_context() {
    if (domain_context != NULL) {
        fflush(domain_context);
        fclose(domain_context);
    }
}

bool seridog_load_domain_context() {
    bool first_try = true;
    LOAD:
    domain_context = fopen(domain_context_loc, "r+");
    // if we could not open it , it's probably because it doesn't exist so we create it
    if (domain_context == NULL && first_try) {
        first_try = false; // this flag is used to make sure we don't make a loop and fail fast
        domain_context = fopen(domain_context_loc, "ab+");
        fclose(domain_context);
        goto LOAD;
    }
    if (domain_context == NULL) return_false_with("could not load domain context")
    return true;
}

int get_domain_line_index(char *domain) {
    fseek(domain_context, 0, SEEK_SET);
    char *kuff;
    int ans = -1;
    while ((kuff = read_next_line_in_heap(domain_context)) != NULL) {
        ans++;
        if (starts_with(kuff, domain)) {
            free(kuff);
            goto RET;
        }
        free(kuff);
    }

    // at at the end
    kuff = malloc(1024);
    fseek(domain_context, 0, SEEK_END);
    memset(kuff, 0, 1024);
    strcat(kuff, domain);
    strcat(kuff, ":\n");
    fputs(kuff, domain_context);
    fflush(domain_context);
    free(kuff);
    ans++;
    RET:
    return ans;
}

bool seridog_add_user_to_domain(char *username, char *domain) {

    if (!is_user_exist(username)) return_false_with("no such user");
    int domain_index = get_domain_line_index(domain);
    char *domain_buff = read_nth_line(domain_context, domain_index + 1);
    domain_buff = realloc(domain_buff, strlen(domain_buff) + 2 * strlen(username) + 15); // make it wide enough!

    char user_name_formated[strlen(username) + 2];
    memset(user_name_formated, 0, strlen(username) + 2);
    strcpy(user_name_formated, username);
    user_name_formated[strlen(username)] = ',';
    char *where_it_exist = strstr(domain_buff, user_name_formated);
    if (where_it_exist != NULL) return true;

    // when it doesn't exist
    // add and persist
    domain_buff[strlen(domain_buff) - 1] = '\0'; // because it's new-line
    strcat(domain_buff, user_name_formated);
    strcat(domain_buff, "\n\0");
    domain_context = replace_line_in_file(domain_context, domain_context_loc, domain_buff, domain_index);
    return true;
}

void seridog_get_domain_info(char ***userNames, int *count, char *domain_name) {
    int i = get_domain_line_index(domain_name);
    if (i == -1) {
        *count = -1; // not found!
        return;
    }

    char *buffer = read_nth_line(domain_context, i + 1);

    size_t trimmed_len = strlen(strstr(buffer, ":") + 1);
    if (trimmed_len == 1) {
        free(buffer);
        *count = -1;
        return;
    }


    char *trimmed_buffer = malloc(trimmed_len);
    strncpy(trimmed_buffer, strstr(buffer, ":") + 1, trimmed_len);
    free(buffer);
    *userNames = split(trimmed_buffer, ',', count);

}


/****** type context ****/
#define type_context_loc "type_context.cv"
FILE *type_context;


bool seridog_load_type_context() {
    bool first_try = true;
    LOAD:
    type_context = fopen(type_context_loc, "r+");
    // if we could not open it , it's probably because it doesn't exist so we create it
    if (type_context == NULL && first_try) {
        first_try = false; // this flag is used to make sure we don't make a loop and fail fast
        type_context = fopen(type_context_loc, "ab+");
        fclose(type_context);
        goto LOAD;
    }
    if (type_context == NULL) return_false_with("could not load type context")
    return true;
}

void seridog_close_type_context() {
    if (type_context != NULL) {
        fflush(type_context);
        fclose(type_context);
    }
}

int get_type_line_index(char *type_name) {
    fseek(type_context, 0, SEEK_SET);
    char *kuff;
    int ans = -1;
    while ((kuff = read_next_line_in_heap(type_context)) != NULL) {
        ans++;
        if (starts_with(kuff, type_name)) {
            free(kuff);
            goto RET;
        }
        free(kuff);
    }

    // at at the end
    kuff = malloc(1024);
    fseek(type_context, 0, SEEK_END);
    memset(kuff, 0, 1024);
    strcat(kuff, type_name);
    strcat(kuff, ":\n");
    fputs(kuff, type_context);
    fflush(type_context);
    free(kuff);
    ans++;
    RET:
    return ans;
}

bool seridog_add_object_to_type(char *type, char *object) {
    int type_index = get_type_line_index(type);
    char *type_buff = read_nth_line(type_context, type_index + 1);
    type_buff = realloc(type_buff, strlen(type_buff) + 2 * strlen(object) + 15); // make it wide enough!

    char user_name_formated[strlen(object) + 2];
    memset(user_name_formated, 0, strlen(object) + 2);
    strcpy(user_name_formated, object);
    user_name_formated[strlen(object)] = ',';
    char *where_it_exist = strstr(type_buff, user_name_formated);
    if (where_it_exist != NULL) return true;

    // when it doesn't exist
    // add and persist
    type_buff[strlen(type_buff) - 1] = '\0'; // because it's new-line
    strcat(type_buff, user_name_formated);
    strcat(type_buff, "\n\0");
    type_context = replace_line_in_file(type_context, type_context_loc, type_buff, type_index);
    return true;
}


void seridog_get_type_info(char ***objects, int *count, char *type_name) {
    int i = get_type_line_index(type_name);
    if (i == -1) {
        *count = -1; // not found!
        return;
    }

    char *buffer = read_nth_line(type_context, i + 1);

    size_t trimmed_len = strlen(strstr(buffer, ":") + 1);
    if (trimmed_len == 1) {
        free(buffer);
        *count = -1;
        return;
    }


    char *trimmed_buffer = malloc(trimmed_len);
    strncpy(trimmed_buffer, strstr(buffer, ":") + 1, trimmed_len);
    free(buffer);
    *objects = split(trimmed_buffer, ',', count);

}

/******** access context ****/

char *serialize_DomainTypeTuple(DomainTypeTuple t) {
    size_t size = strlen(t.type) + strlen(t.domain) + 2;
    char *buff = malloc(size);
    memset(buff, 0, size);
    strcat(buff, t.domain);
    strcat(buff, ",");
    strcat(buff, t.type);

    return buff;
}

#define access_context_loc "access_context.cv"
FILE *access_context;


bool seridog_load_access_context() {
    bool first_try = true;
    LOAD:
    access_context = fopen(access_context_loc, "r+");
    // if we could not open it , it's probably because it doesn't exist so we create it
    if (access_context == NULL && first_try) {
        first_try = false; // this flag is used to make sure we don't make a loop and fail fast
        access_context = fopen(access_context_loc, "ab+");
        fclose(access_context);
        goto LOAD;
    }
    if (access_context == NULL) return_false_with("could not load access context")
    return true;
}

void seridog_close_access_context() {
    if (access_context != NULL) {
        fflush(access_context);
        fclose(access_context);
    }
}

int get_access_line_index(DomainTypeTuple typeTuple) {
    fseek(access_context, 0, SEEK_SET);
    char *serialized_tuple = serialize_DomainTypeTuple(typeTuple);

    char *kuff;
    int ans = -1;
    while ((kuff = read_next_line_in_heap(access_context)) != NULL) {
        ans++;
        if (starts_with(kuff, serialized_tuple)) {
            free(kuff);
            goto RET;
        }
        free(kuff);
    }

    // at at the end
    kuff = malloc(1024);
    fseek(access_context, 0, SEEK_END);
    memset(kuff, 0, 1024);
    strcat(kuff, serialized_tuple);
    strcat(kuff, ":\n");
    fputs(kuff, access_context);
    fflush(access_context);
    free(kuff);
    free(serialized_tuple);
    ans++;
    RET:
    return ans;
}

bool seridog_add_access(char *op, DomainTypeTuple t) {

    int access_index = get_access_line_index(t);
    char *access_buff = read_nth_line(access_context, access_index + 1);
    access_buff = realloc(access_buff, strlen(access_buff) + 2 * strlen(op) + 15); // make it wide enough!

    char op_formated[strlen(op) + 2];
    memset(op_formated, 0, strlen(op) + 2);
    strcpy(op_formated, op);
    op_formated[strlen(op)] = ',';
    char *where_it_exist = strstr(access_buff, op_formated);
    if (where_it_exist != NULL) return true;

    // when it doesn't exist
    // add and persist
    access_buff[strlen(access_buff) - 1] = '\0'; // because it's new-line
    strcat(access_buff, op_formated);
    strcat(access_buff, "\n\0");
    access_context = replace_line_in_file(access_context, access_context_loc, access_buff, access_index);
    return true;
}


/***Can Access a True DatabaseCall using CV files***/

/*
 * let me explain!
 *                                   / Domain -1----+-> User
 * Operation <-+----1- Domain_Type <
 *                                   \Type -1----+-> Object
 *
 * okay, that was time-consuming!
 * each user can be in multiple Domains, (one to many)
 * or in other sense each Domain Contains multiple Users (many to one)
 * similarly each Type Contains multiple Objects (many to one)
 *
 * now suppose we have a hybrid entity called Domain_Type which can be identified uniquely by a domain and a type (one to each pair)
 * and this entity can have multiple Operations
 *
 * this is like Database Entity Relation view of the situation
 *
 * now to answer the question CanAccess(user,obj,op):
 * we have to move backward from User and Object to Domain_Types and then check if any of them contain Operation:
 *
 * types   = select types   where type   contains object
 * domains = select domains where domain contains user
 *
 * Domain_Type = CartesianProduct(types,domains) a.k.a nested for loops
 * check if any Domain_Type contains operation,
 * now please close your eyes on what I'm about to write down below , because it is going to be ugly!
 *
 */
bool canAccess_smaller(char *types, char *domain, char *op) {
    int i = get_access_line_index((DomainTypeTuple){types,domain});
    char* query  = read_nth_line(access_context,i+1);
    size_t op_len = strlen(op);
    char op_formated[op_len+2];
    strcpy(op_formated,op);
    strcat(op_formated,",");

    return strstr(query,op_formated) != NULL;
}

bool canAccess(char *user, char *obj, char *op) {

    // i want to act lazy to be performed, by just querying one single list (say types)
    // and then lazy load each domain and check if i can return this function
    // it's exactly like for-for loop but inner loop will dynamically have IO!

    // read all types from obj

    size_t temp_size = strlen(obj);
    char obj_formated[temp_size + 2];
    strcpy(obj_formated, obj);
    strcat(obj_formated, ",");


    fseek(type_context, 0, SEEK_SET);
    char *type_buff;
    char *types[1024]; // a hardcoded limit of 1024*1024 types can exist for object!
    size_t matching_object_type = 0;

    // i felt a power when i wrote this
    while ((type_buff = read_next_line_in_heap(type_context)) != NULL) {
        if (strstr(type_buff, obj_formated) != NULL) {
            temp_size = strstr(type_buff, ":") - type_buff;
            char *s = malloc(temp_size);
            strncpy(s, type_buff, temp_size);
            types[matching_object_type++] = s;
        }
        free(type_buff);
    }



    // now take out domains out one by one
    temp_size = strlen(user);
    char user_formated[temp_size + 2];
    strcpy(user_formated, user);
    strcat(user, ",");

    fseek(domain_context, 0, SEEK_SET);
    char *domain_buff;
    bool canAcFlag = false;
    while ((domain_buff = read_next_line_in_heap(domain_context)) != NULL && !canAcFlag) {
        if (strstr(domain_buff, user) != NULL) {
            temp_size = strstr(domain_buff, ":") - domain_buff;
            char *matching_domain = malloc(temp_size);
            strncpy(matching_domain, domain_buff, temp_size);
            for (int i = 0; i < matching_object_type; ++i) {
                if (canAccess_smaller(types[i], matching_domain, op)) {
                    canAcFlag = true;
                }
            }
        }
        free(domain_buff);
    }
    for (int i = 0; i < matching_object_type; ++i) {
        free(types[i]);
    }
    return canAcFlag;
//    return false;
}



#endif //SECURITY_SERIDOG_H
