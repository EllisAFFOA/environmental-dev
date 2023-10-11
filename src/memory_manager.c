

/*--------------------------------------------------------------  
-----  INCLUDES  -----------------------------------------------
----------------------------------------------------------------*/

/***************************************************************
 * 
 *  Include zephyr utilities 
 *  
 ***************************************************************/
#include <zephyr/drivers/flash.h>
#include <zephyr/storage/flash_map.h>
#include <zephyr/fs/nvs.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/logging/log.h>

/***************************************************************
 * 
 *  Include src utilities
 *  
 ***************************************************************/
#include "memory_manager.h"



/*--------------------------------------------------------------  
-----  DEFINES  ------------------------------------------------
----------------------------------------------------------------*/

/***************************************************************
 * 
 *  Define module name for debugging    
 *  
 ***************************************************************/
#define MODULE_NAME memory_manager

/***************************************************************
 * 
 *  Define nvs storage partitions
 *  
 ***************************************************************/
#define NVS_PARTITION		    storage_partition
#define NVS_PARTITION_DEVICE	FIXED_PARTITION_DEVICE(NVS_PARTITION)
#define NVS_PARTITION_OFFSET	FIXED_PARTITION_OFFSET(NVS_PARTITION)



/*--------------------------------------------------------------  
-----  ENUMS  --------------------------------------------------
----------------------------------------------------------------*/



/*--------------------------------------------------------------  
-----  STRUCTS  ------------------------------------------------
----------------------------------------------------------------*/

/***************************************************************
 * 
 *  Memory data members for load/save transactions
 *  
 ***************************************************************/
typedef struct memory_container_s {
    char* key;
    uint32_t id;
    uint32_t value;
} memory_container_t;



/*--------------------------------------------------------------  
-----  LOCAL MEMBER ("m_") VARIABLE DECLARATION  ---------------
----------------------------------------------------------------*/

/***************************************************************
 * 
 *  Memory data members for load/save transactions
 *  
 ***************************************************************/
LOG_MODULE_REGISTER(MODULE_NAME, LOG_LEVEL_DBG);

//static const struct device *m_memory_flash_device;                        // flash device 
static struct nvs_fs m_fs;                                                  // nvs file system
static bool m_is_initialized = false;                                       // initialization check
static memory_container_t m_items[] = {                                     // local data storage
    {"test_val_1", 1, 123}, {"test_val_2", 2, 123},
    {"test_val_3", 3, 123}}; 
                 




/*--------------------------------------------------------------  
-----  GLOBAL VARIABLE DECLARATION  ----------------------------
----------------------------------------------------------------*/
size_t g_num_items = sizeof(m_items) / sizeof(memory_container_t);   // number of data members   



/*--------------------------------------------------------------  
-----  GLOBAL VARIABLE REFS  -----------------------------------
----------------------------------------------------------------*/



/*--------------------------------------------------------------  
-----  GLOBAL VARIABLE REFS  -----------------------------------
----------------------------------------------------------------*/



/*--------------------------------------------------------------  
-----  LOCAL FUNCTION PROTOTYPES  ------------------------------
----------------------------------------------------------------*/
size_t _find_item_index(const char* key);


/*==============================================================
=====  Public APIs  ============================================
================================================================*/

/***************************************************************
 * 
 * @brief Initialization of Memory Manager
 *
 * Mount flash driver and initialize nvs. Set default parameter 
 * values if they do not yet exist in memory space.
 *
 * @param None
 *
 * @retval 0 returned if successful.
 * @retval Flash related errors of device binding or flash get page info fails.
 * @retval NVS error if nvs_init fails.
 * 
 ***************************************************************/
int memory_manager_init(void) 
{
    int err;
    struct flash_pages_info flash_info;

    // check if memory_manager is initialized alread
    if (m_is_initialized) {
        return 0;
    }

    // set up flash device
    m_fs.flash_device = NVS_PARTITION_DEVICE;
    if (!device_is_ready(m_fs.flash_device)) {
        LOG_WRN("Flash Device %s Not Ready", m_fs.flash_device->name);
        return MEMORY_ERR_FLASH_NOT_READY;
    }
    
    // get flash offset
    m_fs.offset = NVS_PARTITION_OFFSET;
    err = flash_get_page_info_by_offs(m_fs.flash_device, m_fs.offset, &flash_info);
    if (err) {
        LOG_WRN("Can't Get Flash Page Info");
        return MEMORY_ERR_FLASH_PAGE;
    }
    m_fs.sector_size = flash_info.size;
    m_fs.sector_count = 2U;

    // mount nvs device
    err = nvs_mount(&m_fs);
    if (err) {
        LOG_WRN("NVS Flash Mount Failed");
        return MEMORY_ERR_FLASH_MOUNT;
    }

    // get values stored in memory, or set default values if none exist
    for (size_t i=0; i < g_num_items; i++) {
        LOG_DBG("m_items[%u] : {key = %s, id = %u, value = %u}", i, m_items[i].key, m_items[i].id, m_items[i].value);
        err = nvs_read(&m_fs, m_items[i].id, &m_items[i].value, sizeof(m_items[i].value));
        if (err <= 0) {
            nvs_write(&m_fs, m_items[i].id, &m_items[i].value, sizeof(m_items[i].value));
        }
    }

    // memory_manager initialization complete
    m_is_initialized = true;
    return MEMORY_SUCCESS;
}


/***************************************************************
 * 
 * @brief Load value from nvs
 *
 * Use key to load value from memory and store it to data container
 * indicated by pointer.
 * 
 * @param[in]  key      Key of requested data
 * @param[out] data_p   Pointer to memory that data will be loaded into 
 * @param[in]  data_len Size of memory that data will be loaded into
 *
 * @retval 0 is returned if the param_id parameter was copied to the memory
 * space provided.
 * @retval The error returned by TestDataMemory(), if an error is detected.
 * 
 ***************************************************************/
int memory_load(const char* key, uint8_t *data_p, size_t data_len) 
{
    size_t index;

    // make sure nvs initialized
    if (!m_is_initialized) {
        LOG_WRN("NVS Not Initialized, Unable to Load");
        return MEMORY_ERR_NOT_INITIALIZED;
    }

    // get data index from key
    index = _find_item_index(key);
    if (index < 0) {
        LOG_WRN("Data Member [%s] Does Not Exist", key);
        return MEMORY_ERR_INVALID_KEY;
    }

    // load data from local struct to desired location
    memcpy(data_p, (uint8_t *)&m_items[index].value, data_len);
    return MEMORY_SUCCESS;
}


/***************************************************************
 * 
 * @brief Save value to nvs

 * Use key to save value to specified nvs location
 * 
 * @param[in]  key      Key of requested data
 * @param[out] data_p   Pointer to data that will be saved
 * @param[in]  data_len Size of memory that data will be saved
 *
 * @retval If the data pointer and size is correct, a positive integer offset
 * to the param_id in the m_param_data structure is returned.
 * @retval -EFAULT if the data pointer is NULL.
 * @retval -EINVAL if data_size in not correct for the given param_id
 * 
 ***************************************************************/
int memory_save(const char* key, uint8_t *data_p, size_t data_len) 
{
    int err;
    size_t index;
    
    // make sure nvs initialized
    if (!m_is_initialized) {
        LOG_WRN("NVS Not Initialized, Unable to Save");
        return MEMORY_ERR_NOT_INITIALIZED;
    }
    
    // get data index from key
    index = _find_item_index(key);
    if (index < 0) {
        LOG_WRN("Data Member [%s] Does Not Exist", key);
        return MEMORY_ERR_INVALID_KEY;
    }

    // save data from desired location to local struct
    memcpy((uint8_t *)&m_items[index].value, data_p, data_len);

    // write data to nvs and compare against number of bytes written (returned by func)
    // if not equal to 0 or length of data, an error has occured
    err = nvs_write(&m_fs, m_items[index].id, &m_items[index].value, data_len);
    if (err == 0 || err == data_len) {
        return MEMORY_SUCCESS;
    }
    else {
        return MEMORY_ERR_WRITE;
    }
}


/***************************************************************
 * 
 * @brief Access nvs functionality
 * Save or load by indicating access method. 
 * MEMORY_SAVE = true, MEMORY_LOAD = false
 * 
 * @param[in]  method   Access method
 * @param[in]  key      Key of requested data
 * @param[out] data_p   Pointer to data that will be saved
 * @param[in]  data_len Size of memory that data will be saved
 *
 * @retval If the data pointer and size is correct, a positive integer offset
 * to the param_id in the m_param_data structure is returned.
 * @retval -EFAULT if the data pointer is NULL.
 * @retval -EINVAL if data_size in not correct for the given param_id
 * 
 ***************************************************************/
int memory_access(int method, const char* key, uint8_t *data_p, size_t data_len) 
{   
    int ret;

    switch (method)
    {
    case MEMORY_LOAD:
        ret = memory_load(key, data_p, data_len);
        break;

    case MEMORY_SAVE:
        ret = memory_save(key, data_p, data_len);
        break;
    
    default:
        ret = MEMORY_ERR_ACCESS_DIR;
        break;
    }

    return ret;
}



/*==============================================================
=====  Private APIs  ===========================================
================================================================*/
/***************************************************************
 * 
 * @brief Find index of item in local data container
 *
 * Use key to save value to specified nvs location
 * 
 * @param[in]  key      Key of requested item
 *
 * @retval Index of data container if it exists
 * @retval 0 if key does not exist
 * 
 ***************************************************************/
size_t _find_item_index(const char* key) {
    for (size_t i=0; i < g_num_items; i++) {
        if (strcmp(m_items[i].key, key) == 0) {
            return i;
        }
    }
    return -1;
}