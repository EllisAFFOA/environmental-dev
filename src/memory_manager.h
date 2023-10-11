#ifndef __MEMORY_MANAGER_H__
#define __MEMORY_MANAGER_H__


/*----  Includes  ------------------------------------------------------------*/

#include "common.h"


/*----  Defines  -------------------------------------------------------------*/
#define MEMORY_SAVE     true
#define MEMORY_LOAD     false

/*----  Enumerations  --------------------------------------------------------*/
typedef enum memory_id_e {
    MEMORY_ID_TEST_1,
    MEMORY_ID_TEST_2,
    MEMORY_ID_TEST_3
} memory_id_t;

typedef enum memory_err_e {
    MEMORY_SUCCESS,
    MEMORY_ERR_ALREADY_INITIALIZED,
    MEMORY_ERR_FLASH_NOT_READY,
    MEMORY_ERR_FLASH_PAGE,
    MEMORY_ERR_FLASH_MOUNT,
    MEMORY_ERR_NOT_INITIALIZED,
    MEMORY_ERR_INVALID_KEY,
    MEMORY_ERR_WRITE,
    MEMORY_ERR_ACCESS_DIR
}memory_err_t;


/*----  Global Variables  ----------------------------------------------------*/
extern size_t g_num_items;

/*----  Structures  ----------------------------------------------------------*/

/*----  Callback Functions  --------------------------------------------------*/

/*----  Function prototypes --------------------------------------------------*/
int memory_manager_init(void);
int memory_load(const char* key, uint8_t *data_p, size_t data_len);
int memory_save(const char* key, uint8_t *data_p, size_t data_len);
int memory_access(int method, const char* key, uint8_t *data_p, size_t data_len);


#endif