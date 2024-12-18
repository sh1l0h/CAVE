#include "graphics/model.h"
#include "core/log.h"
#include "cJSON/cJSON.h"
#include <dirent.h>

HashMap models;

i32 models_init(const char *model_dir_path)
{
    DIR *model_dir = opendir(model_dir_path);
    if(model_dir == NULL) {
        log_fatal("Could not open directory at %s", model_dir_path);
        return 1;
    }

    struct dirent *entry;
    while((entry = readdir(model_dir)) != NULL) {
        if(entry->d_type != DT_REG) {
            log_warn("Non regular file %s in model directory at %s", entry->d_name,
                     model_dir_path);
            continue;
        }

        u64 file_name_size = strlen(entry->d_name);
        if(file_name_size < 5 || strcmp(&entry->d_name[file_name_size - 5], ".json")) {
            log_warn("Non json file %s in model directory at %s", entry->d_name,
                     model_dir_path);
            continue;
        }

        u64 model_dir_path_size = strlen(model_dir_path);
        char *model_file_path = malloc((model_dir_path_size + file_name_size + 1) *
                                       sizeof(char));
        snprintf(model_file_path, model_dir_path_size + file_name_size + 1, "%s/%s",
                 model_dir_path, entry->d_name);

        FILE *model_file = fopen(model_file_path, "r");
        if(model_file == NULL) {


        }

        cJSON *model_json = cJSON_Parse(file_path);
        if(model_json == NULL)
            log_error("%s", cJSON_GetErrorPtr());


    }
}

Model *models_get(const char *model_path)
{
}
