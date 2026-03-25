#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include "pkcs11.h"

#define PKCS11_LIB "/usr/lib/softhsm/libsofthsm2.so"
#define TOKEN_LABEL "MyToken"
#define USER_PIN "5678"

void check_ck_r(CK_RV rv, const char *msg) {
    if (rv != CKR_OK) {
        fprintf(stderr, "ERROR: %s (CK_RV=0x%lX)\n", msg, rv);
        exit(EXIT_FAILURE);
    }
}

int main() {
    void *lib_handle;
    CK_RV (*C_Initialize)(CK_VOID_PTR);
    CK_RV (*C_Finalize)(CK_VOID_PTR);
    CK_RV (*C_GetSlotList)(CK_BBOOL, CK_SLOT_ID_PTR, CK_ULONG_PTR);
    CK_RV (*C_OpenSession)(CK_SLOT_ID, CK_FLAGS, CK_VOID_PTR, CK_NOTIFY, CK_SESSION_HANDLE *);
    CK_RV (*C_CloseSession)(CK_SESSION_HANDLE);
    CK_RV (*C_Login)(CK_SESSION_HANDLE, CK_USER_TYPE, CK_UTF8CHAR_PTR, CK_ULONG);
    CK_RV (*C_Logout)(CK_SESSION_HANDLE);
    CK_RV (*C_FindObjectsInit)(CK_SESSION_HANDLE, CK_ATTRIBUTE_PTR, CK_ULONG);
    CK_RV (*C_FindObjects)(CK_SESSION_HANDLE, CK_OBJECT_HANDLE_PTR, CK_ULONG, CK_ULONG_PTR);
    CK_RV (*C_FindObjectsFinal)(CK_SESSION_HANDLE);

    lib_handle = dlopen(PKCS11_LIB, RTLD_NOW);
    if (!lib_handle) {
        fprintf(stderr, "Failed to load PKCS#11 library: %s\n", dlerror());
        return EXIT_FAILURE;
    }

    // Load required function pointers
    C_Initialize = dlsym(lib_handle, "C_Initialize");
    C_Finalize = dlsym(lib_handle, "C_Finalize");
    C_GetSlotList = dlsym(lib_handle, "C_GetSlotList");
    C_OpenSession = dlsym(lib_handle, "C_OpenSession");
    C_CloseSession = dlsym(lib_handle, "C_CloseSession");
    C_Login = dlsym(lib_handle, "C_Login");
    C_Logout = dlsym(lib_handle, "C_Logout");
    C_FindObjectsInit = dlsym(lib_handle, "C_FindObjectsInit");
    C_FindObjects = dlsym(lib_handle, "C_FindObjects");
    C_FindObjectsFinal = dlsym(lib_handle, "C_FindObjectsFinal");

    if (!C_Initialize || !C_Finalize || !C_GetSlotList || !C_OpenSession ||
        !C_CloseSession || !C_Login || !C_Logout || !C_FindObjectsInit ||
        !C_FindObjects || !C_FindObjectsFinal) {
        fprintf(stderr, "Failed to load PKCS#11 function pointers.\n");
        return EXIT_FAILURE;
    }

    // Initialize PKCS#11
    check_ck_r(C_Initialize(NULL), "C_Initialize failed");

    // Get slots
    CK_SLOT_ID slots[32];
    CK_ULONG slot_count = 32;
    check_ck_r(C_GetSlotList(CK_TRUE, slots, &slot_count), "C_GetSlotList failed");

    CK_SLOT_ID token_slot = 0;
    CK_ULONG i;
    int found = 0;

    for (i = 0; i < slot_count; i++) {
        CK_TOKEN_INFO token_info;
        CK_RV rv = (*(CK_RV(*)(CK_SLOT_ID, CK_TOKEN_INFO_PTR))dlsym(lib_handle, "C_GetTokenInfo"))(slots[i], &token_info);
        if (rv != CKR_OK) continue;

        char label[33];
        memcpy(label, token_info.label, 32);
        label[32] = '\0';
        for (int j = 31; j >= 0 && label[j] == ' '; j--) label[j] = '\0';  // trim spaces

        if (strcmp(label, TOKEN_LABEL) == 0) {
            token_slot = slots[i];
            found = 1;
            break;
        }
    }

    if (!found) {
        fprintf(stderr, "Token '%s' not found!\n", TOKEN_LABEL);
        C_Finalize(NULL);
        return EXIT_FAILURE;
    }

    // Open a session
    CK_SESSION_HANDLE session;
    check_ck_r(C_OpenSession(token_slot, CKF_SERIAL_SESSION | CKF_RW_SESSION, NULL, NULL, &session),
               "C_OpenSession failed");

    // Login
    check_ck_r(C_Login(session, CKU_USER, (CK_UTF8CHAR_PTR)USER_PIN, strlen(USER_PIN)),
               "C_Login failed");

    // List objects
    CK_OBJECT_CLASS obj_class;
    CK_ATTRIBUTE search_template[] = {
        { CKA_CLASS, &obj_class, sizeof(obj_class) }
    };

    check_ck_r(C_FindObjectsInit(session, NULL, 0), "C_FindObjectsInit failed");

    CK_OBJECT_HANDLE objects[64];
    CK_ULONG obj_count;
    check_ck_r(C_FindObjects(session, objects, 64, &obj_count), "C_FindObjects failed");

    printf("Found %lu objects on token '%s':\n", obj_count, TOKEN_LABEL);
    for (CK_ULONG j = 0; j < obj_count; j++) {
        CK_ATTRIBUTE label_attr = { CKA_LABEL, NULL, 0 };
        CK_ATTRIBUTE id_attr = { CKA_ID, NULL, 0 };

        // First get lengths
        (*(CK_RV(*)(CK_SESSION_HANDLE, CK_OBJECT_HANDLE, CK_ATTRIBUTE_PTR, CK_ULONG))dlsym(lib_handle, "C_GetAttributeValue"))(session, objects[j], &label_attr, 1);
        (*(CK_RV(*)(CK_SESSION_HANDLE, CK_OBJECT_HANDLE, CK_ATTRIBUTE_PTR, CK_ULONG))dlsym(lib_handle, "C_GetAttributeValue"))(session, objects[j], &id_attr, 1);

        char label[128] = {0};
        unsigned char id[64] = {0};
        label_attr.pValue = label;
        label_attr.ulValueLen = sizeof(label);
        id_attr.pValue = id;
        id_attr.ulValueLen = sizeof(id);

        (*(CK_RV(*)(CK_SESSION_HANDLE, CK_OBJECT_HANDLE, CK_ATTRIBUTE_PTR, CK_ULONG))dlsym(lib_handle, "C_GetAttributeValue"))(session, objects[j], &label_attr, 1);
        (*(CK_RV(*)(CK_SESSION_HANDLE, CK_OBJECT_HANDLE, CK_ATTRIBUTE_PTR, CK_ULONG))dlsym(lib_handle, "C_GetAttributeValue"))(session, objects[j], &id_attr, 1);

        printf("Object %lu: Label='%s', ID=", j+1, label);
        for (unsigned long k = 0; k < id_attr.ulValueLen; k++)
            printf("%02X", id[k]);
        printf("\n");
    }

    check_ck_r(C_FindObjectsFinal(session), "C_FindObjectsFinal failed");

    // Logout and close
    check_ck_r(C_Logout(session), "C_Logout failed");
    check_ck_r(C_CloseSession(session), "C_CloseSession failed");
    check_ck_r(C_Finalize(NULL), "C_Finalize failed");

    dlclose(lib_handle);
    printf("Done.\n");
    return 0;
}

