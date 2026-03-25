#include <p11-kit/pkcs11.h>
#include <cstring>

namespace {
CK_FUNCTION_LIST g_function_list;
bool g_function_list_ready = false;

void fill_padded(unsigned char *dst, std::size_t n, const char *src) {
    std::memset(dst, ' ', n);
    if (!src) return;
    std::size_t len = std::strlen(src);
    if (len > n) len = n;
    std::memcpy(dst, src, len);
}

void init_function_list();
}

extern "C" {

CK_RV C_Initialize(void *) {
    return CKR_OK;
}

CK_RV C_Finalize(void *) {
    return CKR_OK;
}

CK_RV C_GetInfo(CK_INFO_PTR info) {
    if (!info) return CKR_ARGUMENTS_BAD;
    std::memset(info, 0, sizeof(*info));
    info->cryptokiVersion.major = 2;
    info->cryptokiVersion.minor = 40;
    info->libraryVersion.major = 0;
    info->libraryVersion.minor = 1;
    fill_padded(info->manufacturerID, sizeof(info->manufacturerID), "ToyVendor");
    fill_padded(info->libraryDescription, sizeof(info->libraryDescription), "Toy PKCS11 Library");
    return CKR_OK;
}

CK_RV C_GetSlotList(CK_BBOOL, CK_SLOT_ID_PTR pSlotList, CK_ULONG_PTR pulCount) {
    if (!pulCount) return CKR_ARGUMENTS_BAD;

    if (!pSlotList) {
        *pulCount = 1;
        return CKR_OK;
    }

    if (*pulCount < 1) {
        *pulCount = 1;
        return CKR_BUFFER_TOO_SMALL;
    }

    pSlotList[0] = 1;
    *pulCount = 1;
    return CKR_OK;
}

CK_RV C_GetSlotInfo(CK_SLOT_ID slotID, CK_SLOT_INFO_PTR info) {
    if (!info) return CKR_ARGUMENTS_BAD;
    if (slotID != 1) return CKR_SLOT_ID_INVALID;

    std::memset(info, 0, sizeof(*info));
    fill_padded(info->slotDescription, sizeof(info->slotDescription), "Toy Slot");
    fill_padded(info->manufacturerID, sizeof(info->manufacturerID), "ToyVendor");
    info->flags = CKF_TOKEN_PRESENT;
    info->hardwareVersion.major = 0;
    info->hardwareVersion.minor = 1;
    info->firmwareVersion.major = 0;
    info->firmwareVersion.minor = 1;
    return CKR_OK;
}

CK_RV C_GetTokenInfo(CK_SLOT_ID slotID, CK_TOKEN_INFO_PTR info) {
    if (!info) return CKR_ARGUMENTS_BAD;
    if (slotID != 1) return CKR_SLOT_ID_INVALID;

    std::memset(info, 0, sizeof(*info));
    fill_padded(info->label, sizeof(info->label), "ToyToken");
    fill_padded(info->manufacturerID, sizeof(info->manufacturerID), "ToyVendor");
    fill_padded(info->model, sizeof(info->model), "PKCS11SIM");
    fill_padded(info->serialNumber, sizeof(info->serialNumber), "00000001");
    info->flags = CKF_TOKEN_INITIALIZED | CKF_RNG;
    info->ulMaxSessionCount = CK_EFFECTIVELY_INFINITE;
    info->ulSessionCount = 0;
    info->ulMaxRwSessionCount = CK_EFFECTIVELY_INFINITE;
    info->ulRwSessionCount = 0;
    info->ulMaxPinLen = 255;
    info->ulMinPinLen = 4;
    info->ulTotalPublicMemory = CK_UNAVAILABLE_INFORMATION;
    info->ulFreePublicMemory = CK_UNAVAILABLE_INFORMATION;
    info->ulTotalPrivateMemory = CK_UNAVAILABLE_INFORMATION;
    info->ulFreePrivateMemory = CK_UNAVAILABLE_INFORMATION;
    info->hardwareVersion.major = 0;
    info->hardwareVersion.minor = 1;
    info->firmwareVersion.major = 0;
    info->firmwareVersion.minor = 1;
    return CKR_OK;
}

CK_RV C_OpenSession(CK_SLOT_ID slotID, CK_FLAGS, CK_VOID_PTR, CK_NOTIFY, CK_SESSION_HANDLE_PTR phSession) {
    if (!phSession) return CKR_ARGUMENTS_BAD;
    if (slotID != 1) return CKR_SLOT_ID_INVALID;
    *phSession = 1;
    return CKR_OK;
}

CK_RV C_CloseSession(CK_SESSION_HANDLE) {
    return CKR_OK;
}

CK_RV C_CloseAllSessions(CK_SLOT_ID slotID) {
    if (slotID != 1) return CKR_SLOT_ID_INVALID;
    return CKR_OK;
}

CK_RV C_GetSessionInfo(CK_SESSION_HANDLE hSession, CK_SESSION_INFO_PTR pInfo) {
    if (!pInfo) return CKR_ARGUMENTS_BAD;
    if (hSession != 1) return CKR_SESSION_HANDLE_INVALID;

    std::memset(pInfo, 0, sizeof(*pInfo));
    pInfo->slotID = 1;
    pInfo->state = CKS_RO_PUBLIC_SESSION;
    pInfo->flags = CKF_SERIAL_SESSION;
    pInfo->ulDeviceError = 0;
    return CKR_OK;
}

CK_RV C_GetFunctionList(CK_FUNCTION_LIST_PTR_PTR ppFunctionList) {
    if (!ppFunctionList) return CKR_ARGUMENTS_BAD;
    init_function_list();
    *ppFunctionList = &g_function_list;
    return CKR_OK;
}

} // extern "C"

namespace {
void init_function_list() {
    if (g_function_list_ready) return;

    std::memset(&g_function_list, 0, sizeof(g_function_list));
    g_function_list.version.major = 2;
    g_function_list.version.minor = 40;

    g_function_list.C_Initialize = C_Initialize;
    g_function_list.C_Finalize = C_Finalize;
    g_function_list.C_GetInfo = C_GetInfo;
    g_function_list.C_GetFunctionList = C_GetFunctionList;
    g_function_list.C_GetSlotList = C_GetSlotList;
    g_function_list.C_GetSlotInfo = C_GetSlotInfo;
    g_function_list.C_GetTokenInfo = C_GetTokenInfo;
    g_function_list.C_OpenSession = C_OpenSession;
    g_function_list.C_CloseSession = C_CloseSession;
    g_function_list.C_CloseAllSessions = C_CloseAllSessions;
    g_function_list.C_GetSessionInfo = C_GetSessionInfo;

    g_function_list_ready = true;
}
}
